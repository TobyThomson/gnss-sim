# GNSS-SIM: Tool to generate realistic GNSS-signals
% TODO: What is the tool, what is it inspired? How does it work? Where is it up to? For what purpose was it created?

## Installation Instructions
% TODO: Neaten this up a fair bit
% Linux instructions only
% Need to get RTKLIB and progressbar libs (not included to avoid upstream changes being overlooked)
% Distribution of RTKLIB is a little confusion.
% Need to get version "2.4.3 b34" from GitHub to have the lastest version (https://github.com/tomojitakasu/RTKLIB/tree/rtklib_2.4.3).
% Library is included in the project and then makefile compiles and links the bits it needs
% Need to get progressbar lib from here: https://github.com/doches/progressbar

## Usage
% TODO: Add content here about how to use the tool
### Ephermemeris Download Instructions
% Daily ephemerides are provided for free by the CDDIS/IGS: https://igs.org/products-access/#gnss-broadcast-ephemeris
% More info on getting the daily broadcast info: https://cddis.nasa.gov/Data_and_Derived_Products/GNSS/broadcast_ephemeris_data.html
% European provider of multi gnss data: https://igs.bkg.bund.de/searchRINEX
% RINEX (.rx) files often distributed as compressed .crx files. Can be uncompressed with this tool: https://terras.gsi.go.jp/ja/crx2rnx.html

## TODO:
- [ ] Get pseudorange calculations working...
- [ ] Set carrier phase starting points at correct time
- [ ] Populate outstanding navmessage fields (mostly to do with atmospheric corrections)
- [ ] Calculate signal attenuations
- [ ] Speed up channel allocation navbit and ca chip pointer setting
- [ ] Resolve weird bug where GNSS-SDR won't spit out a "gps_ephemeris.xml" file for some svs. Not sure if this is something to do with IODC or sv generation (e. Block III vs IIR)