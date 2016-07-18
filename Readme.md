# Motorcycle Lean Angle Video Overlay Generator

This program takes an nmea data file produced by a GPS as
input and generates PNG image frames that can be imported to a video
editing program and overlaid on on-board footage from a helmet camera
or similar.

There are many smartphone apps available that will record GPS
information in nmea format.

## Building

### Dependencies

 - Cairo, [cairographics.org](https://cairographics.org).
 - Make, [Gnu Make](https://gnu.org/software/make).

If the dependencies are present then `make` should be all you need to
do.

## Use

`./mct mylog.nmea`

PNG images (video frames) will be generated in the current directory.

Overlaying the frames on video from a GoPro or similar can be done
with most video editing packages. I use [OpenShot](www.openshot.org)

## Limitations aka todo's:

   - GPS data is assumed to be recorded at 1Hz (1 location fix per
     second)
   - A gauge representing lean angle is the only available output.
   - If the GPS signal is lost and picked up again the time elapsed
     between the last good location fix and the time when the next
     good fix is achieved is assumed to be 1 second! This will give
     some very interesting results but will only last for one
     data point (frame).
   - Overlay graphics are white in colour.
   - Frame size of output is 200x200 pixels.
   - A red synchronisation light will flash twice at the start of the
     video. This is for use with some hardware I am building.

## Note about the calculation

The calculation of lean angle is based only on speed and track bearing
recorded by a GPS. Therefore the motorcycle must be in motion for any
non-zero value of lean angle to be recorded. The position of the GPS
recording device on the rider or the bike has a negligible effect on
the resulting estimation of lean angle. 
