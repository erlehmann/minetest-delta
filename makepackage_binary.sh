#!/bin/sh

PROJECT_NAME=minetest-delta
PACKAGEDIR=../$PROJECT_NAME-packages
PACKAGENAME=$PROJECT_NAME-binary-`date +%y%m%d%H%M%S`
PACKAGEPATH=$PACKAGEDIR/$PACKAGENAME

mkdir -p $PACKAGEPATH
mkdir -p $PACKAGEPATH/bin
mkdir -p $PACKAGEPATH/data
mkdir -p $PACKAGEPATH/doc
mkdir -p $PACKAGEPATH/locale/de/LC_MESSAGES/

cp minetest.conf.example $PACKAGEPATH/

cp bin/$PROJECT_NAME.exe $PACKAGEPATH/bin/
cp bin/Irrlicht.dll $PACKAGEPATH/bin/
cp bin/zlibwapi.dll $PACKAGEPATH/bin/
#cp bin/test $PACKAGEPATH/bin/
#cp bin/fasttest $PACKAGEPATH/bin/
#cp bin/server $PACKAGEPATH/bin/
#cp ../irrlicht/irrlicht-1.7.1/lib/Linux/libIrrlicht.a $PACKAGEPATH/bin/
#cp ../jthread/jthread-1.2.1/src/.libs/libjthread-1.2.1.so $PACKAGEPATH/bin/

cp -r data/fontlucida.png $PACKAGEPATH/data/
cp -r data/player.png $PACKAGEPATH/data/
cp -r data/player_back.png $PACKAGEPATH/data/
cp -r data/stone.png $PACKAGEPATH/data/
cp -r data/grass.png $PACKAGEPATH/data/
cp -r data/grass_footsteps.png $PACKAGEPATH/data/
cp -r data/water.png $PACKAGEPATH/data/
cp -r data/tree.png $PACKAGEPATH/data/
cp -r data/leaves.png $PACKAGEPATH/data/
cp -r data/mese.png $PACKAGEPATH/data/
cp -r data/cloud.png $PACKAGEPATH/data/
cp -r data/sign.png $PACKAGEPATH/data/
cp -r data/sign_back.png $PACKAGEPATH/data/
cp -r data/rat.png $PACKAGEPATH/data/
cp -r data/mud.png $PACKAGEPATH/data/
cp -r data/torch.png $PACKAGEPATH/data/
cp -r data/torch_on_floor.png $PACKAGEPATH/data/
cp -r data/torch_on_ceiling.png $PACKAGEPATH/data/
cp -r data/tree_top.png $PACKAGEPATH/data/
cp -r data/coalstone.png $PACKAGEPATH/data/
cp -r data/crack.png $PACKAGEPATH/data/
cp -r data/wood.png $PACKAGEPATH/data/
cp -r data/stick.png $PACKAGEPATH/data/
cp -r data/tool_wpick.png $PACKAGEPATH/data/
cp -r data/tool_stpick.png $PACKAGEPATH/data/
cp -r data/tool_mesepick.png $PACKAGEPATH/data/
cp -r data/grass_side.png $PACKAGEPATH/data/
cp -r data/lump_of_coal.png $PACKAGEPATH/data/
cp -r data/lump_of_iron.png $PACKAGEPATH/data/
cp -r data/mineral_coal.png $PACKAGEPATH/data/
cp -r data/mineral_iron.png $PACKAGEPATH/data/
cp -r data/sand.png $PACKAGEPATH/data/

#cp -r data/pauseMenu.gui $PACKAGEPATH/data/

cp -r doc/README.txt $PACKAGEPATH/doc/README.txt

cp -r locale/de/LC_MESSAGES/$PROJECT_NAME.mo $PACKAGEPATH/locale/de/LC_MESSAGES/

cd $PACKAGEDIR
rm $PACKAGENAME.zip
zip -r $PACKAGENAME.zip $PACKAGENAME

