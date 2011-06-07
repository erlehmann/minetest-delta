/*
Minetest-c55
Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#include "clouds.h"
#include "noise.h"
#include "constants.h"
#include "debug.h"

Clouds::Clouds(
		scene::ISceneNode* parent,
		scene::ISceneManager* mgr,
		s32 id,
		float cloud_y,
		u32 seed
):
	scene::ISceneNode(parent, mgr, id),
	m_cloud_y(cloud_y),
	m_seed(seed),
	m_camera_pos(0,0),
	m_time(0)
{
	dstream<<__FUNCTION_NAME<<std::endl;

	m_material.setFlag(video::EMF_LIGHTING, false);
	m_material.setFlag(video::EMF_BACK_FACE_CULLING, false);
	m_material.setFlag(video::EMF_BILINEAR_FILTER, false);
	m_material.setFlag(video::EMF_FOG_ENABLE, false);
	//m_material.setFlag(video::EMF_ANTI_ALIASING, true);
	//m_material.MaterialType = video::EMT_TRANSPARENT_VERTEX_ALPHA;

	m_box = core::aabbox3d<f32>(-BS*1000000,cloud_y-BS,-BS*1000000,
			BS*1000000,cloud_y+BS,BS*1000000);

}

Clouds::~Clouds()
{
	dstream<<__FUNCTION_NAME<<std::endl;
}

void Clouds::OnRegisterSceneNode()
{
	if(IsVisible)
	{
		//SceneManager->registerNodeForRendering(this, scene::ESNRP_TRANSPARENT);
		SceneManager->registerNodeForRendering(this, scene::ESNRP_SOLID);
	}

	ISceneNode::OnRegisterSceneNode();
}

#define MYROUND(x) (x > 0.0 ? (int)x : (int)x - 1)

void Clouds::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();

	/*if(SceneManager->getSceneNodeRenderPass() != scene::ESNRP_TRANSPARENT)
		return;*/
	if(SceneManager->getSceneNodeRenderPass() != scene::ESNRP_SOLID)
		return;

	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
	driver->setMaterial(m_material);
	
	/*
		Clouds move from X+ towards X-
	*/

	const s16 cloud_radius_i = 12;
	const float cloud_size = BS*50;
	const v2f cloud_speed(-BS*2, 0);
	
	// Position of cloud noise origin in world coordinates
	v2f world_cloud_origin_pos_f = m_time*cloud_speed;
	// Position of cloud noise origin from the camera
	v2f cloud_origin_from_camera_f = world_cloud_origin_pos_f - m_camera_pos;
	// The center point of drawing in the noise
	v2f center_of_drawing_in_noise_f = -cloud_origin_from_camera_f;
	// The integer center point of drawing in the noise
	v2s16 center_of_drawing_in_noise_i(
		MYROUND(center_of_drawing_in_noise_f.X / cloud_size),
		MYROUND(center_of_drawing_in_noise_f.Y / cloud_size)
	);
	// The world position of the integer center point of drawing in the noise
	v2f world_center_of_drawing_in_noise_f = v2f(
		center_of_drawing_in_noise_i.X * cloud_size,
		center_of_drawing_in_noise_i.Y * cloud_size
	) + world_cloud_origin_pos_f;

	for(s16 zi=-cloud_radius_i; zi<cloud_radius_i; zi++)
	for(s16 xi=-cloud_radius_i; xi<cloud_radius_i; xi++)
	{
		v2s16 p_in_noise_i(
			xi+center_of_drawing_in_noise_i.X,
			zi+center_of_drawing_in_noise_i.Y
		);

		/*if((p_in_noise_i.X + p_in_noise_i.Y)%2==0)
			continue;*/
		/*if((p_in_noise_i.X/2 + p_in_noise_i.Y/2)%2==0)
			continue;*/

		v2f p0 = v2f(xi,zi)*cloud_size + world_center_of_drawing_in_noise_f;
		
		double noise = noise2d_perlin_abs(
				(float)p_in_noise_i.X*cloud_size/BS/200,
				(float)p_in_noise_i.Y*cloud_size/BS/200,
				m_seed, 3, 0.4);
		if(noise < 0.8)
			continue;
		
		v2f p1 = p0 + v2f(1,1)*cloud_size;

		//video::SColor c(128,255,255,255);
		float b = m_brightness;
		video::SColor c(128,b*230,b*230,b*255);
		video::S3DVertex vertices[4] =
		{
			video::S3DVertex(p0.X,m_cloud_y,p0.Y, 0,0,0, c, 0,1),
			video::S3DVertex(p0.X,m_cloud_y,p1.Y, 0,0,0, c, 1,1),
			video::S3DVertex(p1.X,m_cloud_y,p1.Y, 0,0,0, c, 1,0),
			video::S3DVertex(p1.X,m_cloud_y,p0.Y, 0,0,0, c, 0,0),
		};
		u16 indices[] = {0,1,2,2,3,0};
		driver->drawVertexPrimitiveList(vertices, 4, indices, 2,
				video::EVT_STANDARD, scene::EPT_TRIANGLES, video::EIT_16BIT);
	}
}

void Clouds::step(float dtime)
{
	m_time += dtime;
}

void Clouds::update(v2f camera_p, float brightness)
{
	m_camera_pos = camera_p;
	m_brightness = brightness;
}

