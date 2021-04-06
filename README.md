# SimpleVoronoi
Draw a simple Voronoi diagram art from any image to the terminal using block graphics. Computed vonoroi sites by setting thredshold to flood fill and group pixels in similar region to the same centroid.

# TerminalImageViewer

I borrowed some code from https://github.com/stefanhaustein/TerminalImageViewer to display block graphics in the terminal 

# Build

```
git clone https://github.com/ctkhanhly/SimpleVoronoi.git 
cd SimpleVoronoi 
make 
sudo make install 
simple_vonoroi -w [width] -h [height] -d [distance thredshold for flood_fill] [file_name]
```


