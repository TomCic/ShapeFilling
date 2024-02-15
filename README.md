## Introduction

This project contains functionality that could be applied in the Monster Mash project at later date. Current project is able to export Monster Mash project zip file,
containing all neccessary files for successfully loading the 3D model in the monstermash.zone website.

## License

The source code in the src folder is licensed under the MIT license. All source files and libraries in the dependencies folder and the packages folder
and their subdirectories are licensed under their own licences which may be more restrictive.

## Building

The project was created on the Windows 10 platform, using the MSVC compiler and C++17. There are solution and project files in the root directory of the project.
Include Allegro 5 NuGet package in the Visual Studio and the eigen library to the dependencies folder.

## Usage and IO

Testing images are in the pictures folder. Another images can be obtained from the addresses listed in the _info.txt file.
Type the name of the image you want to use to the console. The loaded image should not use alpha channel, as the aplha channel will affect the resulting color. The image must be placed 
in the **/pictures** folder before loading it with the program. If you are not sure where to place the image, start the program. The program will create the folder for images in the directory 
where your executable is placed, or in your project folder. Move your picture there and type its name to the console to load it. All output image files and intermediate images will be placed
in the **/pictures** folder. All saved binary files will be placed in the **/saves** folder, containing all progress you have saved. Use the modes and controls to get your desired results.
There are tools available to use the original intensity (grayscale) image without resorting to resetting the whole program. The original image will be ALWAYS scaled and padded to fit
the Monster Mash import resolution. The original image file will not be changed by this operation.
//
View:\
The working window has three view modes available. The mode where the colored original image without both arrows and scribbles can be seen will be the final template texture. Switch between 
them using the A key.
//
Output images and debug images:\
Those files are created upon saving for improvement purposes. When the result in Monster Mash is not satisfactory, user can look up these images and see, which part needs improvement.
The original image should not be overwritten, if its name does not correspond to the names below. The names for output in the program are currently:\
*name of the image* + _mod.png\
screen.png\
depth.png\
segment#.png (# contains any number)\
segments#.png (# contains any number)\
scribbles.png\
scribbleData.png\
//
Improvements:\
Not all attempts at recreating the overlapped/hidden parts will be successful. The reason is that the whole program is heavily influenced by the input provided, and so is the final result.
In cases, where Monster Mash does not accept the generated project data, or when the result is not satisfactory, search in Monster Mash for spaces in segments and other abnormalities. 
The result can be modified further in this program until the desired result is achieved, if possible. Furthermore, you can check the generated project (.zip file) and its contents, which
can be modified manually. Segmentation result (the drawing phase) can be checked upon saving. Files *segment#.png* contain segmentation result for every segmented part of the image.
//
Monster Mash input:\
Monster Mash does not handle most cases, where the grey (merging) parts are overlapped and when there are small (grey) gaps in boundary of any segment.

## Modes

There are curently four modes in the program to create the 3D mesh and segment the image.

### Draw mode

This is the default mode. It uses the input from the mouse to create scribbles on the image. It uses then the Lazy Brush algorithm to fill each area in the image
with the color of the scribbles. Each scribble indicates separate part of the image (should image contain a human, two different scribbles in his/her hand would
indicate, that the arm is composed of two different parts). Shift + LMB is used to apply scribbles of the last active scribble, i.e. the scribble that was drawn
last, or picked in the color picking mode (more in the picking mode and the controls). This is used to correct small mistakes that could happen due
to the placement and size of the already drawn scribbles.

Should any scribble be erased by overwriting it, the application will reset the coloring and depth ordering. This process is done due to the management of the 
overwritten segments and their scribbles. As such, it is highly recommended to finish segmenting, i.e. coloring the image, before continuing with the other operations.

### Depth mode

In this mode, create depth order between the body parts by placing arrows, forming a graph. The image must be already divided to segments by the draw mode. To place 
the arrow, select first the segment that is further. Then click on the segment that is closer. These two clicks will create an arrow that shows depth dependency between the
two segments.

There are currently two arrow types. The green arrow indicates that two neighboring segments will merge in the final 3D model. The red arrow indicates
that the two neighboring segments are related in depth, but the overlapped part will not be merged together with the closer segment.

### Selection mode

This mode was added due to the limitations of the previous one. The red arrow disables merge of two segments. However, there is no way how to disable only part of the
segment from merging that way alone, or how to select part of the boundary for merging. Two clicks in this mode will select a rectangular area where merging of any segments will be disabled or forced. There can be more than
one such area. The second clik determines the area type.

### Segment picking mode

As written above, shift + LMB in depth mode allows creating more scribbles belonging to one segment, the last one used by default. With this tool, the pixel
belonging to the segment that was created earlier can be selected by LMB and drawn in draw mode with shift + LMB.

## Controls:

Q / ESC - quit the application\
C       - change of the scribble/segment color, via UI. If UI is not working, type a new color to the console. (draw mode only)\
H       - switch between soft and hard scribbles (draw mode only)\
D       - switch to the depth mode from any other mode, or enter the draw mode from the depth mode\
P       - enter the segment picking mode from any other mode, or draw mode from the segment picking mode\
V       - enter the selection mode from any other mode, or enter the draw mode from the selection mode\
S       - save progress in the binary file and the current segmentation layers, the scribbles, the (modified) original image, the screen and the depth order in image files\
X       - load progress\
B       - blur image for better segmentation results (draw mode only)\
K       - increase contrast for better segmentation results (draw mode only)\
R       - reset the application\
M       - start the segmentation process\
O       - start approximating the borders of the segments and create the Monster Mash project zip file \
A       - switch controlling dispay of the user input and the segmented image including the original image, segmented result including original image, and segmented result alone\
LMB     - in the draw mode: draw scribbles; in the depth mode: click one time for selecting further segment, second time is for closer segment to add green arrow; in the selection mode: select area where will be no merging of any segments\
RMB     - in the draw mode: draw background scribble in draw mode; in the selection mode: boundaries of all segments in selected area will be set to merge\
sh+D    - reset depth\
sh+V    - reset the selected areas\
sh+R    - reset the grayscale original image only\
sh+LMB  - in the draw mode: draw another scribble for the last segment; in the depth mode: same as LMB, but with the red arrow\



