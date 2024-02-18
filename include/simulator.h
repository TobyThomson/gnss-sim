#ifndef H_SIMULATOR
#define H_SIMULATOR

#define SUBFRAME_COUNT          (5UL)
#define WORD_COUNT              (10)
#define WORD_BIT_COUNT          (30)

#define CA_REGISTER_LENGTH      (10)
#define CA_CHIPS_PER_NAV_BIT    (20)
#define CA_CODE_CHIP_DURATION_S (1.0 / 1023000.0)

#define CA_CODE_SEQUENCE_LENGTH (1023)

// TODO: Add missing references
#define TLM_WORD_PREAMBLE       (0b10001011)
#define TLM_WORD_TLM_MESSAGE    (0b00000000000000) // Don't think we'll need more than this (Ref: IS-GPS-200N, 20.3.3.1 Telemetry Word)
#define TLM_WORD_ISF_LEGACY     (0b0)
#define TLM_WORD_ISF_ENHANCED   (0b1)

#define HOW_WORD_ALERT_NOMINAL  (0b0)
#define HOW_WORD_ALERT_WARNING  (0b1)
#define HOW_WORD_ANTI_SPOOF_ON  (0b1)
#define HOW_WORD_ANTI_SPOOF_OFF (0b0)

// URA ranges below are rounded to 0 d.p (Ref: IS-GPS-200, 20.3.3.3.1.3 SV Accuracy)
#define URA_INDEX_3M            (0UL)
#define URA_INDEX_4M            (1UL)
#define URA_INDEX_5M            (2UL)
#define URA_INDEX_7M            (3UL)
#define URA_INDEX_10M           (4UL)
#define URA_INDEX_14M           (5UL)
#define URA_INDEX_24M           (6UL)
#define URA_INDEX_48M           (7UL)
#define URA_INDEX_96M           (8UL)
#define URA_INDEX_192M          (9UL)
#define URA_INDEX_384M          (10UL)
#define URA_INDEX_768M          (11UL)
#define URA_INDEX_1536M         (12UL)
#define URA_INDEX_3072M         (13UL)
#define URA_INDEX_6144M         (14UL)
#define URA_INDEX_UNAVAILABLE   (15UL)

#define LNAV_DATA_STRUCTURE_ID  (1UL)  // (Ref: )

#define SUBFRAME_4_PAGE_18_ID   (56UL) // (Ref: )
#define SUBFRAME_5_PAGE_25_ID   (51UL) // (Ref: )

#define SAMPLE_FREQUENCY_MSPS   (2.5)
#define SAMPLE_DURATION_S       (30.0)
#define SAMPLE_INTERVAL_S       (1.0 / (SAMPLE_FREQUENCY_MSPS * 1000000.0))
#define IQ_SAMPLE_WINDOW_S      (0.1)
#define CHANNEL_COUNT			(1)

// Multiplied by two as we need to record both and I and a Q value per sample
#define IQ_BUFFER_SIZE          (int)((SAMPLE_FREQUENCY_MSPS * 1000000 * IQ_SAMPLE_WINDOW_S) * 2)

typedef struct {
    char caCodeSequence[CA_CODE_SEQUENCE_LENGTH];
    unsigned long navFrameBoilerPlate[SUBFRAME_COUNT][WORD_COUNT];
	unsigned long navFrame[SUBFRAME_COUNT][WORD_COUNT];
	unsigned long previousWord;
	unsigned short prn;
    unsigned long wn;
    unsigned long tow;
} SV;

typedef struct {
	SV sv;
	int navBitPointer;
    double caChipPointer;
    short caCycleCount;
    short navBit;
} Channel;

void simulate(void (*dumpCallback)(short*, int));

#endif