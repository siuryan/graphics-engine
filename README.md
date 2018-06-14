# graphics-engine
Ryan Siu and Connie Lei, period 4<br>
Computer Graphics MKS66 Final Project

## Added Features
- Lighting
  - `ambient`, `light`, `constants`
  - allow an MDL programmer to set the ambient light, create lighting constants and set multiple point light sources
- Shading
  - more to be added
- Polygon meshes
  - `mesh`
  - allow an MDL programmer to specify a polygon mesh defined in an external OBJ file
  
## Instructions

To compile to an executable, run: 
```bash
$ make all
```

After compilation, to run the graphics engine on a script:
```bash
$ ./mdl <script>
```

For example,
```bash
$ ./mdl robot.mdl
```

To compile and run the graphics engine on a pre-specified script (cow.mdl), and display the animation:
```bash
$ make
$ animate cow.gif
```
