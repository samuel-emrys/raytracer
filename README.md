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
build/build_subfolder/src/raytracing > image.ppm
```

# Open the image

On Linux, you can run

```
xdg-open image.ppm
```
