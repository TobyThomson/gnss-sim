#ifndef H_SIMULATOR
#define H_SIMULATOR

#include <stdbool.h>

// *** SIMULATION CONFIGURATION VALUES ****
#define VISIBILITY_UPDATE_INTERVAL_S    (5.0 * 60.0)
#define SAMPLE_FREQUENCY_MSPS           (4.0)
#define SAMPLE_DURATION_S               (45.0)
#define IQ_SAMPLE_WINDOW_S              (0.1)
#define CHANNEL_COUNT			        (12)
#define CARRIER_PHASE_RESOLUTION_INDEX  (12)
/// *** END ***

#define LIGHTSPEED              (2.99792458e8)
#define SECONDS_IN_WEEK         (60 * 60 * 24 * 7)

#define SUBFRAME_COUNT          (5)
#define WORD_COUNT              (10)
#define WORD_BIT_COUNT          (30)

#define CA_REGISTER_LENGTH      (10)
#define CA_CYCLES_PER_NAV_BIT   (20)
#define CA_CODE_SEQUENCE_LENGTH (1023)
#define SUBFRAME_DURATION_S     (6.0)
#define TOW_RESOLUTION_S        (1.5)

#define CARRIER_FREQUENCY_HZ    (1575420000.0)
#define CA_CODE_FREQUENCY_HZ    (1023000.0)

#define TLM_WORD_PREAMBLE       (0b10001011)
#define TLM_WORD_TLM_MESSAGE    (0b00000000000000)
#define TLM_WORD_ISF_LEGACY     (0b0)
#define TLM_WORD_ISF_ENHANCED   (0b1)
#define HOW_WORD_ALERT_NOMINAL  (0b0)
#define HOW_WORD_ALERT_WARNING  (0b1)
#define HOW_WORD_ANTI_SPOOF_ON  (0b1)
#define HOW_WORD_ANTI_SPOOF_OFF (0b0)
#define LNAV_DATA_STRUCTURE_ID  (1UL)
#define SUBFRAME_4_PAGE_18_ID   (56UL)
#define SUBFRAME_5_PAGE_25_ID   (51UL)

#define CARRIER_WAVELENGTH_M        (LIGHTSPEED / CARRIER_FREQUENCY_HZ)
#define CA_CODE_WAVELENGTH_M        (LIGHTSPEED / CA_CODE_FREQUENCY_HZ)
#define CA_CODE_CHIP_DURATION_S     (1.0 / CA_CODE_FREQUENCY_HZ)
#define FRAME_BIT_COUNT             (SUBFRAME_COUNT * WORD_COUNT * WORD_BIT_COUNT)
#define FRAME_CA_CHIP_COUNT         (FRAME_BIT_COUNT * CA_CYCLES_PER_NAV_BIT * CA_CODE_SEQUENCE_LENGTH)
#define SAMPLE_INTERVAL_S       (1.0 / (SAMPLE_FREQUENCY_MSPS * 1000000.0))
#define TRIG_TABLE_SIZE         (1 << CARRIER_PHASE_RESOLUTION_INDEX)

// For generating bitmasks
#define BITMASK(bits) ((1 << (bits)) - 1)

// Buffer size multiplied by two as we need to record both and I and a Q value per sample
#define IQ_BUFFER_SIZE          (int)((SAMPLE_FREQUENCY_MSPS * 1000000 * IQ_SAMPLE_WINDOW_S) * 2)

typedef struct {
    char caCodeSequence[CA_CODE_SEQUENCE_LENGTH];
    unsigned long navFrameBoilerPlate[SUBFRAME_COUNT][WORD_COUNT];
    unsigned long navFrame[SUBFRAME_COUNT][WORD_COUNT];

	unsigned short prn;

    eph_t ephemeris;
    
    double clockBias_s;
    double variance;

    double position_ecef[3];

    double psuedorange_m;
    double psuedorangeRate_ms;
    double elevation_rad;
} SV;

typedef struct {
	SV* sv;

    double carrierDopplerShift_Hz;
    double codeDopplerShift_Hz;

    double carrierPhase_cycles;
    double codeFrequency_Hz;

    unsigned long previousWord;

    char codeChip;
    char navBit;

    double codeChipPointer;
    int navBitPointer;
} Channel;

void simulate(void (*dumpCallback)(short*, int), eph_t* ephemerides, short svCount);

#endif