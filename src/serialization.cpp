/*
Minetest-c55
Copyright (C) 2010 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "serialization.h"
#include "utility.h"
#ifdef _WIN32
	#define ZLIB_WINAPI
#endif
#include "zlib.h"

/* report a zlib or i/o error */
void zerr(int ret)
{   
    fputs("zerr: ", stderr);
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
		break;
	default:
		dstream<<"return value = "<<ret<<"\n";
    }
}

void compressZlib(SharedBuffer<u8> data, std::ostream &os)
{
	z_stream z;
	const s32 bufsize = 16384;
	//char input_buffer[bufsize];
	char output_buffer[bufsize];
	int input_i = 0;
	int status = 0;
	int ret;

	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;

	ret = deflateInit(&z, -1);
	if(ret != Z_OK)
		throw SerializationError("compressZlib: deflateInit failed");
	
	z.avail_in = 0;
	
	for(;;)
	{
		int flush = Z_NO_FLUSH;
		z.next_out = (Bytef*)output_buffer;
		z.avail_out = bufsize;

		if(z.avail_in == 0)
		{
			//z.next_in = (char*)&data[input_i];
			z.next_in = (Bytef*)&data[input_i];
			z.avail_in = data.getSize() - input_i;
			input_i += z.avail_in;
			if(input_i == (int)data.getSize())
				flush = Z_FINISH;
		}
		if(z.avail_in == 0)
			break;
		status = deflate(&z, flush);
		if(status == Z_NEED_DICT || status == Z_DATA_ERROR
				|| status == Z_MEM_ERROR)
		{
			zerr(status);
			throw SerializationError("compressZlib: deflate failed");
		}
		int count = bufsize - z.avail_out;
		if(count)
			os.write(output_buffer, count);
	}

	deflateEnd(&z);

}

void compressZlib(const std::string &data, std::ostream &os)
{
	SharedBuffer<u8> databuf((u8*)data.c_str(), data.size());
	compressZlib(databuf, os);
}

void decompressZlib(std::istream &is, std::ostream &os)
{
	z_stream z;
	const s32 bufsize = 16384;
	char input_buffer[bufsize];
	char output_buffer[bufsize];
	int status = 0;
	int ret;
	int bytes_read = 0;
	int input_buffer_len = 0;

	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;

	ret = inflateInit(&z);
	if(ret != Z_OK)
		throw SerializationError("dcompressZlib: inflateInit failed");
	
	z.avail_in = 0;
	
	//dstream<<"initial fail="<<is.fail()<<" bad="<<is.bad()<<std::endl;

	for(;;)
	{
		z.next_out = (Bytef*)output_buffer;
		z.avail_out = bufsize;

		if(z.avail_in == 0)
		{
			z.next_in = (Bytef*)input_buffer;
			input_buffer_len = is.readsome(input_buffer, bufsize);
			z.avail_in = input_buffer_len;
			//dstream<<"read fail="<<is.fail()<<" bad="<<is.bad()<<std::endl;
		}
		if(z.avail_in == 0)
		{
			//dstream<<"z.avail_in == 0"<<std::endl;
			break;
		}
			
		//dstream<<"1 z.avail_in="<<z.avail_in<<std::endl;
		status = inflate(&z, Z_NO_FLUSH);
		//dstream<<"2 z.avail_in="<<z.avail_in<<std::endl;
		bytes_read += is.gcount() - z.avail_in;
		//dstream<<"bytes_read="<<bytes_read<<std::endl;

		if(status == Z_NEED_DICT || status == Z_DATA_ERROR
				|| status == Z_MEM_ERROR)
		{
			zerr(status);
			throw SerializationError("decompressZlib: inflate failed");
		}
		int count = bufsize - z.avail_out;
		//dstream<<"count="<<count<<std::endl;
		if(count)
			os.write(output_buffer, count);
		if(status == Z_STREAM_END)
		{
			//dstream<<"Z_STREAM_END"<<std::endl;
			
			//dstream<<"z.avail_in="<<z.avail_in<<std::endl;
			//dstream<<"fail="<<is.fail()<<" bad="<<is.bad()<<std::endl;
			// Unget all the data that inflate didn't take
			for(u32 i=0; i < z.avail_in; i++)
			{
				is.unget();
				if(is.fail() || is.bad())
				{
					dstream<<"unget #"<<i<<" failed"<<std::endl;
					dstream<<"fail="<<is.fail()<<" bad="<<is.bad()<<std::endl;
					throw SerializationError("decompressZlib: unget failed");
				}
			}
			
			break;
		}
	}

	inflateEnd(&z);
}

void compress(SharedBuffer<u8> data, std::ostream &os, u8 version)
{
	if(version >= 11)
	{
		compressZlib(data, os);
		return;
	}

	if(data.getSize() == 0)
		return;
	
	// Write length (u32)

	u8 tmp[4];
	writeU32(tmp, data.getSize());
	os.write((char*)tmp, 4);
	
	// We will be writing 8-bit pairs of more_count and byte
	u8 more_count = 0;
	u8 current_byte = data[0];
	for(u32 i=1; i<data.getSize(); i++)
	{
		if(
			data[i] != current_byte
			|| more_count == 255
		)
		{
			// write count and byte
			os.write((char*)&more_count, 1);
			os.write((char*)&current_byte, 1);
			more_count = 0;
			current_byte = data[i];
		}
		else
		{
			more_count++;
		}
	}
	// write count and byte
	os.write((char*)&more_count, 1);
	os.write((char*)&current_byte, 1);
}

void decompress(std::istream &is, std::ostream &os, u8 version)
{
	if(version >= 11)
	{
		decompressZlib(is, os);
		return;
	}

	// Read length (u32)

	u8 tmp[4];
	is.read((char*)tmp, 4);
	u32 len = readU32(tmp);
	
	// We will be reading 8-bit pairs of more_count and byte
	u32 count = 0;
	for(;;)
	{
		u8 more_count=0;
		u8 byte=0;

		is.read((char*)&more_count, 1);
		
		is.read((char*)&byte, 1);

		if(is.eof())
			throw SerializationError("decompress: stream ended halfway");

		for(s32 i=0; i<(u16)more_count+1; i++)
			os.write((char*)&byte, 1);

		count += (u16)more_count+1;

		if(count == len)
			break;
	}
}


