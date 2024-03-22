# GNSS-SIM: Tool to generate realistic GNSS-signals
## TODO:
- [x] Generate new navmessage for satellite if required
- [x] Work out what C/A code we're on for satellite
- [x] Generate IQ sample!
- [x] Generate C/A codes
- [x] Read generated IQ data with gnss-sdr!
- [x] Modify to work with multiple satellites!
- [x] Get realtime (zmq) streaming working
- [x] Write test cases
- [x] Implement command line argument parsing
- [x] Implement RTKLIB
- [x] Implement RINEX file parsing
- [x] Populate navmessages with values from supplied RINEX file
- [x] Get all the navmessage terms formatted correctly
- [x] Populate the navmessages of the svs correctly
- [x] Align sv time with simulation time
- [x] Determine sv positions n simulation
- [x] Determine if SVs are visible or not
- [x] Implement SV tracking logic
- [x] Add simulation progress bar
- [x] Calculate psuedoranges
- [x] Resolve potential bug where rankings change significantly after being initially established
- [x] Switch to using channels for keeping track of all transmission information
- [x] Calculate sv code phases

- [ ] Calculate sv carrier phases
- [ ] Get GNSS-SDR decoding multiple navigation messages at once
- [ ] Demonstrate MVP working!

- [ ] Calculate sv attenuations
- [ ] Populate outstanding navmessage fields (mostly to do with atmospheric corrections)

## Notes:
* Had to move ```range``` and ```geoid``` variables from bottom to top of file in rtklib's ```geoid.c``` to get it to compile (those variables initiated twice otherwise). Not sure how anyone else gets away with compiling that
* Have resorted to just compiling the rtklib source files that I actually nedd rather than all of them as it appears some of them depends on their own external libraries which linking to will be a pain. This means that ```geoid.c``` file I modified previously is a non-issue for now (I'm not using it. Yet...)
* Interesting bug for later: when decoding Block III GPS navmessages, GNSS-SDR won't spit out a "gps_ephemeris.xml" file. It will when decoding Block IIR navmessages.

* In a GPS reciever, position and time are solved simulatenously by making measurements of the psuedoranges and combining this with the supplementary timing information.
* The GPS satellite clocks are extremly well aligned(!) with their carrier signals and navigation messages almost perfectly in phase at transmission.

* The psuedoranges are established by measuring the navigation message transit times. Each navigationmessage subframe includes the start time of the following subframe. This timing information from each visible satellite is recieved at different times by the reciver. This allows the difference in transit times between the satellites to be established. This can be plugged into a simultaneous equation to determine the actual time at the reciever and it's position.

* Assuming no Doppler shift (for now), each satellite's navigation message would arrive at the reciever in phase with it's carrier. Each satellite's recieved carrier (and thus navigationmessage) would therfore be out of phase.
* We could simulate this using multiple 1.5 GHz oscillators with adjustable phase(one for each simulated satellite), each modulated by a single in-phase navigationmessage. This would require a lot of expensive hardware.
* A functionally identical solution is to use a single 1.5 GHz carrier and just adjust the phase of our navigationmessages relative to the simulated transit time of each signal.
* We simply take the psuedorange and divide it by the speed of light to get the desired time delay, then we phase shift our modulated signal by this value!

* Now let's talk about Doppler. Each freqeuncy in a signal will be Doppler shifted by a different amount, all based on the relative speed between the signal source and the observer. In this case, the carrier frequency will expereince a different amount of doppler shift from the freqeuncies used to express the navigationmessage. This effect may be called "Doppler Spread" (I'm not 100% this is the correct term).
* For now, we will assume that the psudorange rates (sv-reciever relative velocities) are at a level that results in little difference across the baseband navigationmessage spectrum (~24 MHz bandwidth). This should be checked using real world data.
* However, the navigation message carrier frequency is almost 1000x the navigationmessage frequency. We should expect to see considerable relative doppler shift between these frequencies
* We need to calculate the expected relative Doppler shift and apply this to the navigation message signal to simulate this effect (again, phase shifting the navigation messages since we have a fixed carrier source).
* If we do these two things, we should be golden!

* GPS carrier doppler freqeuncies are ~2-5 kHz. Source: Google Images
* GPS carrier doppler freqeuncy rates peak at ~0.6 Hz/s. Source: Google Images

* Soooo, I need to:
1. Calculate the carrier phase offset due to transmission delay
2. Calculate the code-carrier phase offset due to doppler
3. Apply the transimission delay offset as a delay in the code phase (phase won't change as quickly so can be recomputed less often)
4. Apply the doppler offset as a steadily changing phase during the IQ sample generation (phase changes fairly quickly so must be recomputed often)

Questions:
* How often should we recompute the code phase offset? Anwser: As often as is required to trick the reciever into thinking it's looking at the real deal. May be able to work this out by looking at the typical positioning noise on the reciever.
* What resolution do the sin and cos tables need to be for this application? Is there a nice way of regenerating these at compile time

* CARRIER PHASE IS INDEPENDANT OF CODE PHASE! TWO SINUSOIDS WHICH TRAVEL DIFFERENT DISTANCES WILL BE OUT OF PHASE AT THE RECIEVER DESPITE A LACK OF MODULATION. TREAT THE TIME DELAY AND PHASE OFFSET COMPONENTS INDIVIDUALLY!


## Documentation Notes:
* Be sure to talk about why certain design decisions were made