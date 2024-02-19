# GNSS-SIM: Tool to generate realistic GNSS-signals
## TODO:
[x] Generate new navmessage for satellite if required
[x] Work out what C/A code we're on for satellite
[x] Generate IQ sample!
[x] Generate C/A codes
[x] Read generated IQ data with gnss-sdr!
[x] Modify to work with multiple satellites!
[x] Get realtime (zmq) streaming working
[x] Write test cases
[x] Implement command line argument parsing
[ ] Implement RTKLIB
[ ] Implement RINEX file parsing

[ ] Get all the navmessage terms formatted correctly
[ ] Align sv time with simulation time
[ ] Which satellites visible?
[ ] Calculate visible satellite distances to receiver
[ ] Calculate visbile satellite attenuations + phase angle

## Notes:
* Had to move ```range``` and ```geoid``` variables from bottom to top of file in rtklib's ```geoid.c``` to get it to compile (those variables initiated twice otherwise). Not sure how anyone else gets away with compiling that
* Have resorted to just compiling the rtklib source files that I actually nedd rather than all of them as it appears some of them depends on their own external libraries which linking to will be a pain.