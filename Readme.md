# Motorcycle Lean Angle Video Overlay Generator

This program takes an nmea data file produced by a GPS or any log file
with NMEA formatted data as input and generates PNG image frames that
can be imported to a video editing program and overlaid on on-board
footage from a helmet camera or similar.

There are many smartphone apps available that will record GPS
information in nmea format.

![Screenshot](screenshots/demo.png?raw=true)

## Building

### Dependencies

 - Cairo, [cairographics.org](https://cairographics.org).
 - Make, [Gnu Make](https://gnu.org/software/make).

If the dependencies are present then `make` should be all you need to
do.

## Use

`./mct mylog.nmea datarate`

Where mylog.nmea is the log file with NMEA data, and 'datarate' is the
rate of the GPS. If no datarate is specified, a default value of 5Hz
is used.

E.g.

`./mct LOG1.log 5`

PNG images (video frames) will be generated in the current directory.

Overlaying the frames on video from a GoPro or similar can be done
with most video editing packages. I use [OpenShot](www.openshot.org)

## Limitations aka todo's:

   - A gauge representing lean angle is the only available output.
   - If the GPS signal is lost and picked up again the time elapsed
     between the last good location fix and the time when the next
     good fix is achieved is assumed to be 1 second! This will give
     some very interesting results but will only last for one
     data point (frame).
   - Overlay graphics are white in colour.
   - A red synchronisation light will flash twice at the start of the
     video. This is for use with some hardware I am building.

## Note about the calculation

The calculation of lean angle is based only on speed and track bearing
recorded by a GPS module. Therefore the motorcycle must be in motion
for any non-zero value of lean angle to be recorded. The position of
the GPS recording device on the rider or the bike has a negligible
effect on the resulting estimation of lean angle.
