# Ray Tracer

This is a basic ray tracer, developed as an exercise following the [Ray Tracing In One Weekend Tutorial](https://raytracing.github.io/books/RayTracingInOneWeekend.html).

# Output

![](img/output.png)

# Build

```
conan install . -if build --build=missing
conan build . -bf build
```

# Run the program

```
./build/build_subfolder/src/raytracing
```

This will create the file `image.ppm` in your current working directory.

# Open the image

On Linux, you can run

```
xdg-open image.ppm
```
