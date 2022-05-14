# Ray Tracer

This is a basic ray tracer, developed as an exercise following the [Ray Tracing In One Weekend Tutorial](https://raytracing.github.io/books/RayTracingInOneWeekend.html).

# Output

![](img/output.png)

# Build

```bash
conan install . -if build --build=missing
conan build . -bf build
```

# Run the program

```bash
./build/build_subfolder/src/raytracing --help
A simple program to generate a static scene of spheres using a raytracer
Usage: ./build/build_subfolder/src/raytracing [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -o,--output TEXT            The name of the file in which to store the output [image.ppm].
  -f,--format TEXT            The format of the image to output [ppm].
  -a,--aspect-ratio FLOAT     The aspect ratio of the image [1.7777777777777777].
  -w,--width UINT             The width of the image [1200].
  -s,--samples-per-pixel UINT The number of samples to take for each pixel [500].
  -d,--max-depth UINT         The maxmimum number of ray bounces to compute [50].
  -v,--vertical-field-of-view FLOAT
                              The vertical field of view with which to view the scene, in degrees [20].
```

To execute this with default options, simply execute the following:

```bash
./build/build_subfolder/src/raytracing
```

This will create the file `image.ppm` in your current working directory.

# Open the image

On Linux, you can run

```bash
xdg-open image.ppm
```
