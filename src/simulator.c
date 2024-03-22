#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../libs/progressbar/progressbar.h"
#include "../libs/rtklib-2.4.3/src/rtklib.h"

#include "../include/simulator.h"
#include "../include/trig-tables.h"

// Return a long representing the x multiples of "scale factor" required to express the original double.
// (This is the technique used in the GPS spec to encode doubles in the navmessage)
long scaleDouble(double operand, double scaleFactorIndex) {
    double scaleFactor = powf(2, scaleFactorIndex);
    return (long)(operand / scaleFactor);
}

void generateNAVFrameBoilerplate(unsigned long frame[SUBFRAME_COUNT][WORD_COUNT], eph_t* ephemeris) {
    // TODO: Populate missing fields (those that are not provided by eph_t)

    // *** SUBFRAME 1 PARAMETERS ***
    // NOTES:
    // 1. (unsigned) cast required to resolve odd behaviour where compiler was treating 
    //    unsigned long as long during later right-shift operations.
    unsigned long l2Code     = (unsigned)(ephemeris->code                                    & BITMASK(2));
    unsigned long l2flag     = (unsigned)(ephemeris->flag                                    & BITMASK(1));
    unsigned long svAccuracy = (unsigned)(ephemeris->sva                                     & BITMASK(4));
    unsigned long svHealth   = (unsigned)(ephemeris->svh                                     & BITMASK(6));
    unsigned long iodc       = (unsigned)(ephemeris->iodc                                    & BITMASK(10));
    unsigned long tgd        = (unsigned)(scaleDouble(ephemeris->tgd[0],                -31) & BITMASK(8));
    unsigned long toc        = (unsigned)(scaleDouble(time2gpst(ephemeris->toc, NULL),    4) & BITMASK(16));
    unsigned long af2        = (unsigned)(scaleDouble(ephemeris->f2,                    -55) & BITMASK(8));
    unsigned long af1        = (unsigned)(scaleDouble(ephemeris->f1,                    -43) & BITMASK(16));
    unsigned long af0        = (unsigned)(scaleDouble(ephemeris->f0,                    -31) & BITMASK(22));

    // *** EPHEMERIS PARAMETERS ***
    // NOTES:
    // 1. Some values divided by PI to convert from radians to semicircles. Navmessages use
    //    the semicircle unit however the RINEX format (which the supplied ephemeris is in)
    //    stipulates radians be used instead.
    // 2. (unsigned) cast required to resolve odd behaviour where compiler was treating 
    //    unsigned long as long during later right-shift operations.
    unsigned long iode       = (unsigned)(ephemeris->iode                                    & BITMASK(8));
    unsigned long crs        = (unsigned)(scaleDouble(ephemeris->crs,                    -5) & BITMASK(16));
    unsigned long deltaN     = (unsigned)(scaleDouble((ephemeris->deln / PI),           -43) & BITMASK(16));
    unsigned long m0         = (unsigned)(scaleDouble((ephemeris->M0 / PI),             -31) & BITMASK(32));
    unsigned long cuc        = (unsigned)(scaleDouble(ephemeris->cuc,                   -29) & BITMASK(16));
    unsigned long e          = (unsigned)(scaleDouble(ephemeris->e,                     -33) & BITMASK(32));
    unsigned long cus        = (unsigned)(scaleDouble(ephemeris->cus,                   -29) & BITMASK(16));
    unsigned long sqrtA      = (unsigned)(scaleDouble(sqrt(ephemeris->A),               -19) & BITMASK(32));
    unsigned long toe        = (unsigned)(scaleDouble(time2gpst(ephemeris->toe, NULL),    4) & BITMASK(16));
    unsigned long cic        = (unsigned)(scaleDouble(ephemeris->cic,                   -29) & BITMASK(16));
    unsigned long omega0     = (unsigned)(scaleDouble((ephemeris->OMG0 / PI),           -31) & BITMASK(32));
    unsigned long cis        = (unsigned)(scaleDouble(ephemeris->cis,                   -29) & BITMASK(16));
    unsigned long i0         = (unsigned)(scaleDouble((ephemeris->i0 / PI),             -31) & BITMASK(32));
    unsigned long crc        = (unsigned)(scaleDouble(ephemeris->crc,                    -5) & BITMASK(16));
    unsigned long omega      = (unsigned)(scaleDouble((ephemeris->omg / PI),            -31) & BITMASK(32));
    unsigned long omegaDot   = (unsigned)(scaleDouble((ephemeris->OMGd / PI),           -43) & BITMASK(24));
    unsigned long idot       = (unsigned)(scaleDouble((ephemeris->idot / PI),           -43) & BITMASK(14));

    // *** IONOSPHERIC PARAMETERS ***
    // TODO: Correct these
    unsigned long alpha0     = 0;
    unsigned long alpha1     = 0;
    unsigned long alpha2     = 0;
    unsigned long alpha3     = 0;
    unsigned long beta0      = 0;
    unsigned long beta1      = 0;
    unsigned long beta2      = 0;
    unsigned long beta3      = 0;

    // *** UTC PARAMETERS ***
    // TODO: Correct these
    unsigned long a0         = 0;
    unsigned long a1         = 0;
    unsigned long deltaTls   = 0;
    unsigned long tot        = 0;
    unsigned long wnt        = 0;
    unsigned long wnlsf      = 0;
    unsigned long dn         = 0;
    unsigned long deltaTlsf  = 0;

    // *** ALMANAC PARAMETERS ***
    // TODO: Correct these
    unsigned long toa        = 0;
    unsigned long wna        = 0;

    // NOTES:
    // 1. Parity bits computed and populated later for all words
    // 2. "& 0x3FF" = "% 1024"! It's a performance thing (Ref: https://mziccard.me/2015/05/08/modulo-and-division-vs-bitwise-operations/)
    // 3. For the same reason as above, "& 0xFF" = "% 256"!

    // The first two words (TLM and HOW) are almost identical between subframes so generate them all at once
    for (unsigned long subframe = 0; subframe < SUBFRAME_COUNT; subframe++) {
        // NOTES:
        // 1. Using legacy ISF until someone suggests otherwise
        frame[subframe][0] = (TLM_WORD_PREAMBLE << 22) | (TLM_WORD_TLM_MESSAGE << 8) | (TLM_WORD_ISF_LEGACY << 7);

        // NOTES:
        // 1. TOW-Count Message populated later
        // 2. Assuming User Range Accuracy (URA) is normal (HOW_WORD_ALERT_NOMINAL)
        // 3. Bits 23 and 24 generated later during parity bit computation (Ref: IS-GPS-200N, Figure 20-2. TLM and HOW Formats)
        frame[subframe][1] = (HOW_WORD_ALERT_NOMINAL << 12) | (HOW_WORD_ANTI_SPOOF_OFF << 11) | ((subframe + 1) << 8);
    }

    // *** SUBFRAME 1 ***
    // NOTES:
    // 1. Transmission Week Number populated later
    // 2. IODC >> 8 in order to extract just the two MSBs
    frame[0][2] = (l2Code << 18) | (svAccuracy << 14) | (svHealth << 8) | ((iodc >> 8) << 6);

    frame[0][3] = (l2flag << 29);
    frame[0][4] = (0UL);
    frame[0][5] = (0UL);
    frame[0][6] = (tgd << 6);
    frame[0][7] = (iodc << 22) | (toc << 6);
    frame[0][8] = (af2 << 22) | (af1 << 6);

    // NOTES:
    // 1. Bits 23 and 24 generated later during parity bit computation (Ref: IS-GPS-200N, Figure 20-1. Data Format (sheet 1 of 11))
    frame[0][9] = (af0 << 8);

    // *** SUBFRAME 2 ***
    frame[1][2] = (iode << 22) | (crs << 6);
    frame[1][3] = (deltaN << 14) | ((m0 >> 24) << 6);
    frame[1][4] = (m0 << 6);
    frame[1][5] = (cuc << 14) | ((e >> 24) << 6);
    frame[1][6] = (e << 6);
    frame[1][7] = (cus << 14) | ((sqrtA >> 24) << 6);
    frame[1][8] = (sqrtA << 6);

    // NOTES:
    // 1. Bits 23 and 24 generated later during parity bit computation (Ref: IS-GPS-200N, 20.3.3.4.1 Content of Subframes 2 and 3)
    frame[1][9] = (toe << 14);

    // *** SUBFRAME 3 ***
    frame[2][2] = (cic << 14) | ((omega0 >> 24) << 6);
    frame[2][3] = (omega0 << 6);
    frame[2][4] = (cis << 14) | ((i0 >> 24) << 6);
    frame[2][5] = (i0 << 6);
    frame[2][6] = (crc << 14) | ((omega >> 24) << 6);
    frame[2][7] = (omega << 6);
    frame[2][8] = (omegaDot << 6);

    // NOTES:
    // 1. Bits 23 and 24 generated later during parity bit computation (Ref: IS-GPS-200N, 20.3.3.4.1 Content of Subframes 2 and 3)
    frame[2][9] = (iode << 22) | (idot << 8);

    // *** SUBFRAME 4 ***
    // NOTES:
    // 1. Almanac and SV health do not seem strictly necessary (Ref: https://electronics.stackexchange.com/questions/207130/gps-almanac-versus-sky-search).
    // 2. Only generating page 18 (for Ionospheric and UTC data). This technique inspired by gps-sdr-sim (Ref: https://github.com/osqzss/gps-sdr-sim/blob/e7b948ff496917ade58ab7f59d448b7ee6e55f43/gpssim.c#L622C2-L622C2)
    // 3. Presuming that at least one page must be provided for subframe 4?
    frame[3][2] = (LNAV_DATA_STRUCTURE_ID << 28) | (SUBFRAME_4_PAGE_18_ID << 22) | (alpha0 << 14) | (alpha1 << 6);
    frame[3][3] = (alpha2 << 22) | (alpha3 << 14) | (beta0 << 6);
    frame[3][4] = (beta1 << 22) | (beta2 << 14) | (beta3 << 6);
    frame[3][5] = (a1 << 6);
    frame[3][6] = ((a0 >> 8) << 6);
    frame[3][7] = (a0 << 22) | (tot << 14) | (wnt << 6);
    frame[3][8] = (deltaTls << 22) | (wnlsf << 14) | (dn << 6);

    // NOTES:
    // 1. Bits 23 and 24 generated later during parity bit computation (Ref: IS-GPS-200N, 20.3.3.5.1 Content of Subframes 4 and 5)
    frame[3][9] = (deltaTlsf << 22);

    // *** SUBFRAME 5 ***
    // NOTES:
    // 1. Almanac and SV health do not seem strictly necessary (Ref: https://electronics.stackexchange.com/questions/207130/gps-almanac-versus-sky-search).
    // 2. Only generating page 25 as this is the one chosen by gps-sdr-sim (Ref: https://github.com/osqzss/gps-sdr-sim/blob/e7b948ff496917ade58ab7f59d448b7ee6e55f43/gpssim.c#L652)
    // 3. Presuming that at least one page must be provided for subframe 5?
    // 4. Not bothering with anything other than the almanac time stuff (again, inspired by gps-sdr-sim)
	frame[4][2] = (LNAV_DATA_STRUCTURE_ID << 28) | (SUBFRAME_5_PAGE_25_ID << 22) | (toa << 14) | (wna << 6);

    // Mask-off MSBs outside the 30 bit word boundaries (causes issues with parity calculation if not done)
    for (int subframe = 0; subframe < 5; subframe++) {
        for (int word = 0; word < 10; word++) {
            frame[subframe][word] &= BITMASK(30);
        }
    }
}

// The following was script-kiddied from here: https://stackoverflow.com/questions/109023/count-the-number-of-set-bits-in-a-32-bit-integer
int countSetBits(unsigned long i) {
    i = i - ((i >> 1) & 0x55555555);                   // Add pairs of bits
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);    // Quads
    i = (i + (i >> 4)) & 0x0F0F0F0F;                   // Groups of 8
    i *= 0x01010101;                                   // Horizontal sum of bytes
    return  i >> 24;                                   // Return just that top byte (after truncating to 32-bit even when int is wider than uint32_t)
}

void computeParity(unsigned long* word, unsigned long* previousWord, unsigned short solveNibs) {
    // NOTES:
    // 1. This function is quite heavy! Don't worry about understanding it perfectly.
    // 2. "IS-GPS-200N, Table 20-XIV. Parity Encoding Equations" is essential reading if you do want to understand what's going on here.
    // 3. Variable names the same as "IS-GPS-200N, Table 20-XIV. Parity Encoding Equations" to make it easier to follow along

    // Prepend 2 LSBs of the previous transmitted word to this word to form "source"
    unsigned long source = *word | ((*previousWord << 30) & 0xC0000000UL);

    // Masking source so we have just b1 --> b24
    unsigned long d = source & 0x3FFFFFC0UL;

    // Accessing D29 and D30 from previous word.
    unsigned long d29Star = (source >> 31) & 0x1UL;
    unsigned long d30Star = (source >> 30) & 0x1UL;

    // The following graciously provided by gps-sdr-sim. Values can be calculated from "IS-GPS-200N, Table 20-XIV. Parity Encoding Equations"
	unsigned long parityBitMasks[6] = { 
		0x3B1F3480UL,
        0x1D8F9A40UL,
        0x2EC7CD00UL,
		0x1763E680UL,
        0x2BB1F340UL,
        0x0B7A89C0UL
    };

    // If instructed, solve non-information bearing bits 23 and 24 to preserve parity check.
    // This is required for words 2 and 10 of each subframe. See comments in "generateNavmessageBoilerplate".
    // See later comments for an explanation of what's going on here.
    if (solveNibs) {        
        if ((d30Star + countSetBits(parityBitMasks[4] & d)) & 1UL) {
            d ^= (1UL << 6);
        }

        if ((d29Star + countSetBits(parityBitMasks[5] & d)) & 1UL) {
            d ^= (1UL << 7);
        }
	}

    unsigned long D = d;

    // The following XORs b1 --> b24 with d30Star (word XOR D30Star will just be word when D30Star is 0)
    if (d30Star) {
        D ^= 0x3FFFFFC0UL;
    }

    // The following lines compute and insert the 6 parity bits at the end of the word.
    // STEPS:
    // 1. Mask b so we're just looking at the relevant message bits (word & parityBitMasks[i])
    // 2. Count total "set" (1) bits between masked word and DxStar (dxStar + countSetBits(y))
    // 3. Modulo 2 the previous step's result (& 1UL). "& 1UL" = "% 2"! It's a performance thing (Ref: https://mziccard.me/2015/05/08/modulo-and-division-vs-bitwise-operations/)
    // 4. Shift the computer parity bit into the correct location in word
    // For why we use steps 2 and 3, see here: https://stackoverflow.com/questions/32747229/fastest-way-to-xor-all-bits-from-value-based-on-bitmask
    D |= ((d29Star + countSetBits(d & parityBitMasks[0])) & 1UL) << 5;
    D |= ((d30Star + countSetBits(d & parityBitMasks[1])) & 1UL) << 4;
    D |= ((d29Star + countSetBits(d & parityBitMasks[2])) & 1UL) << 3;
    D |= ((d30Star + countSetBits(d & parityBitMasks[3])) & 1UL) << 2;
    D |= ((d30Star + countSetBits(d & parityBitMasks[4])) & 1UL) << 1;
    D |= ((d29Star + countSetBits(d & parityBitMasks[5])) & 1UL);

    D &= 0x3FFFFFFFUL;

    *previousWord = D;
    *word = D;
}

void generateNAVFrame(gtime_t initalTime, unsigned long* previousWord, unsigned long frame[SUBFRAME_COUNT][WORD_COUNT], bool init) {
    int wn;
    double tow = time2gpst(initalTime, &wn);

    // Mask Week Number to 10 bits (Ref: IS-GPS-200N: 3.3.4 GPS Time and SV Z-Count)
    unsigned long navmessageWn = wn & BITMASK(10);

    unsigned long navmessageTow;

    // The last word of the previous frame is required to generate the parity bits for this frame.
    // We must generate the last word of the previous frame if one is not available (occurs on init).
    if (init) {
        // Rewind TOW one subframe to generate this at the correct time
        tow -= SUBFRAME_DURATION_S;

        // "tow" is scaled then masked to 19 bits (Ref: IS-GPS-200N: 3.3.4 GPS Time and SV Z-Count)
        navmessageTow = (unsigned)(tow / TOW_RESOLUTION_S) & BITMASK(19);

        for (short word = 0; word < WORD_COUNT; word++) {
            // Insert TOW into the HOW (2nd word of subframe).
			if (word == 1) {
                // Only want the 17 MSBs of "navmessageTow" (Ref: IS-GPS-200N: 20.3.3.2 Handover Word (HOW))
                frame[4][word] |= ((navmessageTow >> 2) << 13);
            }
            
            // solveNibs = "true" when looking at either the 2nd or 10th word.
            computeParity(&frame[4][word], previousWord, ((word == 1) || (word == 9)));
        }
    }

    for (short subframe = 0; subframe < SUBFRAME_COUNT; subframe++) {
        // Increment the TOW between subframes.
        tow += SUBFRAME_DURATION_S;

        // "tow" is scaled then masked to 19 bits (Ref: IS-GPS-200N: 3.3.4 GPS Time and SV Z-Count)
        navmessageTow = (unsigned)(lround(tow / TOW_RESOLUTION_S) & BITMASK(19));

        for (short word = 0; word < WORD_COUNT; word++) {
            // Add Transmission Week Number to the third word of subframe one.
			if ((subframe == 0) && (word == 2)) {
                frame[subframe][word] |= (navmessageWn << 20);
            }

			// Insert TOW into the HOW (2nd word of subframe).
			if (word == 1) {
                // Only want the 17 MSBs of "navmessageTow" (Ref: IS-GPS-200N: 20.3.3.2 Handover Word (HOW))
                frame[subframe][word] |= ((navmessageTow >> 2) << 13);
            }
            
            // solveNibs = "true" when looking at either the 2nd or 10th word.
            computeParity(&frame[subframe][word], previousWord, ((word == 1) || (word == 9)));
        }
    }
}

// Recommended reading for understanding this function:
// 1. IS-GPS-200N, 3.2.1.3 C/A-Code
// 2. IS-GPS-200N, 3.3.2.3 C/A-Code Generation
// 3. https://dsp.stackexchange.com/questions/52810/gps-coarse-acquisition-prn-codes
// 4. https://www.wasyresearch.com/generating-gps-l1-c-a-pseudo-random-noise-prn-code-with-matlab-and-c-c/
void generateCACodeSequence(char caCodeSequence[CA_CODE_SEQUENCE_LENGTH], char prn) {
    // (Ref: IS-GPS-200N, Table 3-Ia. Code Phase Assignments)
    short delays[] = {  5,   6,   7,   8,  17,  18, 139, 140, 141, 251,
                        252, 254, 255, 256, 257, 258, 469, 470, 471, 472,
                        473, 474, 509, 512, 513, 514, 515, 516, 859, 860,
                        861, 862    };
    
    char g1[CA_CODE_SEQUENCE_LENGTH];
    char g2i[CA_CODE_SEQUENCE_LENGTH];
    char gi[CA_CODE_SEQUENCE_LENGTH];

    char g1Register[10];
    char g2iRegister[10];

    // i and j declared here to save recreating them for each loop
    short i, j;

    // Initialize the registers
    for (i = 0; i < CA_REGISTER_LENGTH; i++) {
        // By initializing the registers with -1, multiplying their bits later gives us the desired effect of modulo 2 addition (i.e. XOR)!
        g1Register[i] = -1;
        g2iRegister[i] = -1;
    }

    // Generate the g1 and g2i sequences
    for (i = 0; i < CA_CODE_SEQUENCE_LENGTH; i++) {
        // Save the register outputs
        g1[i] = g1Register[9];
        g2i[i] = g2iRegister[9];

        // Calculate the feedback values
        char c1 = g1Register[2] * g1Register[9];
        char c2 = g2iRegister[1] * g2iRegister[2] * g2iRegister[5] * g2iRegister[7] * g2iRegister[8] * g2iRegister[9];

        // Shift each of the bits one position to the right
        for (short j = (CA_REGISTER_LENGTH - 1); j > 0; j--) {
            g1Register[j] = g1Register[j - 1];
            g2iRegister[j] = g2iRegister[j - 1];
        }

        // Shift the feedback values into each of the registers
        g1Register[0] = c1;
        g2iRegister[0] = c2;
    }

    // Calculate the C/A Code Sequence!
    // Start by defining two indicies, i and j.
    // i starts at 0 as it is used to index the G1 code.
    // j is delayed "delay[prn - 1]" chips behind i in the previous cycle.
    for (i = 0, j = (CA_CODE_SEQUENCE_LENGTH - delays[prn - 1]); i < CA_CODE_SEQUENCE_LENGTH; i++, j++) {
        // We wish to perform G1 XOR G2i (delayed).
        // Multiplying G1 by G2i can give 1 or -1. Taking this from 1 we get either 0 or 2. We divide by 2 to get either 0 or 1 (desired).
        // j % CA_CODE_SEQUENCE_LENGTH ensures j will wrap back to zero when it crosses the sequence boundary.
        caCodeSequence[i] = (1 - g1[i] * g2i[j % CA_CODE_SEQUENCE_LENGTH]) / 2;
    }
}

void printNavmessage(unsigned long frame[SUBFRAME_COUNT][WORD_COUNT]) {
    for (short subframe = 0; subframe < SUBFRAME_COUNT; subframe++) {
        printf("SUBFRAME %i: \n", (subframe + 1));
        for (short word = 0; word < WORD_COUNT; word++) {
            printf("%i\t%030b\n", (word + 1), frame[subframe][word]);
        }

        printf("\n");
    }
}

// TODO: Just use dot product of pointing vector and normal vector instead?
void calculateAngularDistance(double* satellitePosition_ecef, double* receiverPosition_ecef, float* receiverAngularDistance_rad) {
    double lineOfSightVector_ecef[3];
    double receiverPosition_llh[3];
    double azimuthElevation_rad[2];

    geodist(satellitePosition_ecef, receiverPosition_ecef, lineOfSightVector_ecef);
    ecef2pos(receiverPosition_ecef, receiverPosition_llh);
    satazel(receiverPosition_llh, lineOfSightVector_ecef, azimuthElevation_rad);

    // Using a simplified version of the formula from here: https://math.stackexchange.com/questions/2904702/finding-the-angle-between-two-points-given-their-azimuth-and-elevation-angles
    // Assuming that El1 is pi/2 and Az1 is 0 (center of the sky)
    *receiverAngularDistance_rad = acos(sin(azimuthElevation_rad[1]));
}

void updateSatellitePositions(gtime_t time, SV** svs, short svCount, double* receiverPosition_ecef, double timeStep_s) {
    // Required to keep "geodist" happy. Not used beyond that
    double lineOfSightVector_ecef[3];

    double psuedorange_m;
    double psuedorangeRate_ms;

    for (int sv = 0; sv < svCount; sv++) {
        // Update satellite position in the ECEF frame
        eph2pos(time, &svs[sv]->ephemeris, svs[sv]->position_ecef, &svs[sv]->clockBias_s, &svs[sv]->variance);

        // Calculate the angular distance between the satellite and reciever
        calculateAngularDistance(svs[sv]->position_ecef, receiverPosition_ecef, &svs[sv]->recieverAngularDistance_rad);

        // TODO: Update this to include ionoshperic (+ other?) effects
        // Update the current psuedorange
        psuedorange_m = geodist(svs[sv]->position_ecef, receiverPosition_ecef, lineOfSightVector_ecef);

        // Calculate the new psuedorange rate
        psuedorangeRate_ms = ((psuedorange_m - svs[sv]->psuedorange_m) / timeStep_s);

        // Update the current psuedorange and rate
        svs[sv]->psuedorange_m = psuedorange_m;
        svs[sv]->psuedorangeRate_ms = psuedorangeRate_ms;
    }
}

int compareSatelliteVisibility(const void *p1, const void *p2) {
    const SV *q1 = *(const SV **)p1;
    const SV *q2 = *(const SV **)p2;

    // Sort in descending order of distance from sky center
    if (q1->recieverAngularDistance_rad > q2->recieverAngularDistance_rad) return 1;
    if (q1->recieverAngularDistance_rad < q2->recieverAngularDistance_rad) return -1;
    return 0;
}

void updateChannelAllocations(gtime_t time, Channel channels[CHANNEL_COUNT], SV** svs, short svCount, double* receiverPosition_ecef, double timeStep_s) {
    // Determine all sv positions
    updateSatellitePositions(time, svs, svCount, receiverPosition_ecef, timeStep_s);

    // Rank satellites by distance from the receiver's sky center
    qsort(svs, svCount, sizeof(SV*), compareSatelliteVisibility);

    // Assign satellites to channels by most to least visible
    for (int i = 0; i < CHANNEL_COUNT; i++) {
        // Assign the next most visible satellite in the svs list
        channels[i].sv = svs[i];

        // Generate initial navframe
        generateNAVFrame(time, &channels[i].previousWord, channels[i].sv->navFrame, true);
    }
}

// TODO: Allow user to specify reciever position somehow
void updateRecieverPosition(double* receiverPosition_llh, double* receiverPosition_ecef) {
    // Convert latitude and longitude in degrees to radians
    receiverPosition_llh[0] *= (PI / 180);
    receiverPosition_llh[1] *= (PI / 180);

    pos2ecef(receiverPosition_llh, receiverPosition_ecef);
}

void updateChannelModulation(Channel* channel, double timeStep_s) {
    // Update the carrier phase
    channel->carrierPhase_cycles += channel->carrierDopplerShift_Hz * timeStep_s;

    // Keep the carrier phase between 0 and 1
    if (channel->carrierPhase_cycles >= 1.0) {
        channel->carrierPhase_cycles -= floor(channel->carrierPhase_cycles);
    }

    // TODO: Comment here
    else if (channel->carrierPhase_cycles <= 0.0) {
        channel->carrierPhase_cycles += fabs(ceil(channel->carrierPhase_cycles)) + 1;
    }

    // Hold on to the initial code chip pointer for comparison later
    int initialCodeChip = floor(channel->codeChipPointer);

    // Convert code advance in seconds to chips and add to current code phase
    channel->codeChipPointer += (timeStep_s * channel->codeFrequency_Hz);

    // Calculate the change in C/A code chip index
    int caChipPointerDelta = (int)floor(channel->codeChipPointer) - initialCodeChip;

    // If there was a change in the C/A code chip...
    if (caChipPointerDelta) {
        // Wrap codeChipPointer around after each frame to prevent overflow
        if (channel->codeChipPointer >= FRAME_CA_CHIP_COUNT) {
            channel->codeChipPointer -= FRAME_CA_CHIP_COUNT;
        }

        // Get the next C/A Code bit. Use the code phase to provide the offset. Modulo ensures sequence will not overflow
        channel->codeChip = channel->sv->caCodeSequence[((int)floor(channel->codeChipPointer + channel->codePhase_chips) % CA_CODE_SEQUENCE_LENGTH)];

        // Convert code phase to navbit index
        channel->navBitPointer = (int)floor((channel->codeChipPointer + channel->codePhase_chips) / CA_CODE_SEQUENCE_LENGTH / CA_CYCLES_PER_NAV_BIT);

        // Wrap the navbit pointer back around if it exceeds the frame size
        channel->navBitPointer = (channel->navBitPointer % FRAME_BIT_COUNT);

        // Get the current NAV frame bit
        short subframe = channel->navBitPointer / (WORD_COUNT * WORD_BIT_COUNT);
        short word = (channel->navBitPointer % (WORD_COUNT * WORD_BIT_COUNT)) / WORD_BIT_COUNT;
        short bit = channel->navBitPointer % WORD_BIT_COUNT;

        channel->navBit = (channel->sv->navFrame[subframe][word] >> (29 - bit)) & 0x1;
    }
}

void updateChannelPhases(Channel channels[CHANNEL_COUNT], double* receiverPosition_ecef) {
    double codeDopplerShift_Hz;
    double codePhase_s;
    //double carrierCycles;

    for (int channel = 0; channel < CHANNEL_COUNT; channel++) {
        // Calculate the carrier frequency Doppler shift
        // NOTE: See here for explanation: https://gnss-sdr.org/docs/sp-blocks/observables/#pseudorange-rate-measurement
        channels[channel].carrierDopplerShift_Hz = -channels[channel].sv->psuedorangeRate_ms / CARRIER_WAVELENGTH_M;

        // Calculate the carrier frequency Doppler shift
        // NOTE: This is only performed for the fundermental freqeuncy as the expected modulation bandwidth
        //       is ~24 MHz and the Doppler spread over this range should be limited
        codeDopplerShift_Hz = channels[channel].carrierDopplerShift_Hz * (CA_CODE_FREQUENCY_HZ / CARRIER_FREQUENCY_HZ);

        channels[channel].codeFrequency_Hz = CA_CODE_FREQUENCY_HZ + codeDopplerShift_Hz;

        // TODO: Move and uncomment these lines to enable carrier phase measurements!
        // Calculate carrier cycles between reciever and satellite
        //carrierCycles = (channels[channel].sv->psuedorange_m / CARRIER_WAVELENGTH_M);

        // Find where we are within one cycle of the carrier phase
        //channels[channel].carrierPhase_cycles = 0; //(carrierCycles - floor(carrierCycles));

        // Calculate the difference in expected transmission delay
        codePhase_s = (channels[channel].sv->psuedorange_m / LIGHTSPEED);

        // Update channel code phase
        channels[channel].codePhase_chips = (codePhase_s / CA_CODE_CHIP_DURATION_S);
    }
}

void simulate(void (*dumpCallback)(short*, int), eph_t* ephemerides, short svCount) {
    SV svs[svCount];
    Channel channels[CHANNEL_COUNT];
    short iqBuffer[IQ_BUFFER_SIZE];

    // Create array of pointers to SV elements for ranking satellite visibility
    SV** rankedSvs = (SV**)malloc(svCount * sizeof(*rankedSvs));

    // Populate satellite pointer array
    for (int i = 0; i < svCount; i++) {
        rankedSvs[i] = &svs[i];
    }

    // Get week number from toe of the first ephemeris
    int wn;
    time2gpst(ephemerides->toe, &wn);

    // TODO: Allow user to specify simulation start time and handle this properly in generateNAVFrame()
    gtime_t simulationTime = gpst2time(wn, 6);
    gtime_t simulationEndTime = timeadd(simulationTime, SAMPLE_DURATION_S);
    gtime_t visibilityUpdateTime = simulationTime;

    // Coordinates are for University of Leeds, School of Electronic and Electrical Engineering
    double receiverPosition_llh[3] = { 53.8096268, -1.5553807, 5.0 };
    double receiverPosition_ecef[3];

    // Setup satellites
    for (char i = 0; i < svCount; i++) {
        SV sv;
        
        // Set satellite ephemeris
        sv.ephemeris = *(ephemerides + i);
        sv.prn = sv.ephemeris.sat;

        // Set the ephemeris to the simulation start time (ensures the simulation time is ahead of the last reported ephemeris time)
        sv.ephemeris.toe = simulationTime;

        // "The toe shall be equal to the toc of the same LNAV CEI data set" (Ref: 20.3.4.4 Data Sets)
        sv.ephemeris.toc = (ephemerides + i)->toe;

        // Generate miscellaneous data
        generateNAVFrameBoilerplate(sv.navFrameBoilerPlate, &sv.ephemeris);
        memcpy(sv.navFrame, sv.navFrameBoilerPlate, sizeof(sv.navFrameBoilerPlate));
        generateCACodeSequence(sv.caCodeSequence, sv.prn);

        svs[i] = sv;
    }

    // Setup channels
    for (char i = 0; i < CHANNEL_COUNT; i++) {
        Channel channel;

        channel.sv = NULL;
        channel.previousWord = 0;
        channel.codeChip = 0;
        channel.codeChipPointer = 0;
        channel.navBit = 0;
        channel.navBitPointer = 0;
        channel.codePhase_chips = 0;
        channel.carrierPhase_cycles = 0;

        channels[i] = channel;
    }

    // Create progress bar
    unsigned long steps = (SAMPLE_DURATION_S / IQ_SAMPLE_WINDOW_S);
    progressbar *progress = progressbar_new("GENERATING IQ DATA...", steps);

    // Perform simulation!
    while (timediff(simulationEndTime, simulationTime) > 0) {
        updateRecieverPosition(receiverPosition_llh, receiverPosition_ecef);

        // Decide if it's time to update which satellites are in view
        if (timediff(visibilityUpdateTime, simulationTime) <= 0) {
            updateChannelAllocations(simulationTime, channels, rankedSvs, svCount, receiverPosition_ecef, IQ_SAMPLE_WINDOW_S);

            // Stage the next visibility update
            visibilityUpdateTime = timeadd(visibilityUpdateTime, VISIBILITY_UPDATE_INTERVAL_S);
        }

        // ...otherwise just update the visible satellite positions
        else {
            updateSatellitePositions(simulationTime, rankedSvs, CHANNEL_COUNT, receiverPosition_ecef, IQ_SAMPLE_WINDOW_S);
        }

        // Determine satellite-reciever distances and how this impacts the code and carrier phases
        updateChannelPhases(channels, receiverPosition_ecef);

        // Fill sample window IQ buffer
        for (int i = 0; i < IQ_BUFFER_SIZE; i += 2) {
            short iAccumulated = 0;
            short qAccumulated = 0;
            int previousNavBitPointer = 0;
            int carrierPhaseIndex = 0;

            for (char channel = 0; channel < CHANNEL_COUNT; channel++) {
                // Hold on to the current navbit pointer value
                previousNavBitPointer = channels[channel].navBitPointer;

                // Increment the code phase and update the channel modulation. May result in no change.
                updateChannelModulation(&channels[channel], SAMPLE_INTERVAL_S);

                // Map the channel's carrier phase to an index in the carrier phase look-up table
                // NOTE: Using look-up table to save processing time otherwise spent computing trig functions
                carrierPhaseIndex = (int)(channels[channel].carrierPhase_cycles * (TRIG_TABLE_SIZE - 1));

                //printf("%i\n", sinTable[carrierPhaseIndex]);

                // Write I followed by Q value to the buffer
                // NOTES:
                // 1. "((x * 2) - 1)" maps a 0/1 value to a -1/1 value. Multiplying these remapped terms allows us to XOR them
                // 2. Multiplication by sine and cosine used to introduce carrier phase differences
                iAccumulated += ((channels[channel].codeChip * 2) - 1) * ((channels[channel].navBit * 2) - 1) * cosTable[carrierPhaseIndex];
                qAccumulated += ((channels[channel].codeChip * 2) - 1) * ((channels[channel].navBit * 2) - 1) * sinTable[carrierPhaseIndex];

                // If the navbit pointer wrapped around (i.e. we've gone past the last bit of this NAV frame)...
                if (channels[channel].navBitPointer < previousNavBitPointer) {
                    // Generate the next NAV frame!
                    memcpy(channels[channel].sv->navFrame, channels[channel].sv->navFrameBoilerPlate, sizeof(channels[channel].sv->navFrameBoilerPlate));
                    generateNAVFrame(simulationTime, &(channels[channel].previousWord), channels[channel].sv->navFrame, false);
                }
            }

            iqBuffer[i] = iAccumulated;
            iqBuffer[i + 1] = qAccumulated;

            simulationTime = timeadd(simulationTime, SAMPLE_INTERVAL_S);
        }

        dumpCallback(iqBuffer, IQ_BUFFER_SIZE);

        progressbar_inc(progress);
    }

    progressbar_finish(progress);

    free(rankedSvs);
}