#ifndef mini_al_h__
#define mini_al_h__




#ifdef __cplusplus
extern "C" {
#endif


#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4201)   // nonstandard extension used: nameless struct/union
#pragma warning(disable:4324)   // structure was padded due to alignment specifier
#endif

// Platform/backend detection.
#ifdef _WIN32
#define MAL_WIN32
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_PC_APP || WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#define MAL_WIN32_UWP
#else
#define MAL_WIN32_DESKTOP
#endif
#else
#define MAL_POSIX
#include <pthread.h>    // Unfortunate #include, but needed for pthread_t, pthread_mutex_t and pthread_cond_t types.

#ifdef __unix__
#define MAL_UNIX
#if defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define MAL_BSD
#endif
#endif
#ifdef __linux__
#define MAL_LINUX
#endif
#ifdef __APPLE__
#define MAL_APPLE
#endif
#ifdef __ANDROID__
#define MAL_ANDROID
#endif
#ifdef __EMSCRIPTEN__
#define MAL_EMSCRIPTEN
#endif
#endif

#include <stddef.h> // For size_t.

#ifndef MAL_HAS_STDINT
#if defined(_MSC_VER)
#if _MSC_VER >= 1600
#define MAL_HAS_STDINT
#endif
#else
#if defined(__has_include)
#if __has_include(<stdint.h>)
#define MAL_HAS_STDINT
#endif
#endif
#endif
#endif

#if !defined(MAL_HAS_STDINT) && (defined(__GNUC__) || defined(__clang__))   // Assume support for stdint.h on GCC and Clang.
#define MAL_HAS_STDINT
#endif

#ifndef MAL_HAS_STDINT
typedef   signed char               mal_int8;
typedef unsigned char               mal_uint8;
typedef   signed short              mal_int16;
typedef unsigned short              mal_uint16;
typedef   signed int                mal_int32;
typedef unsigned int                mal_uint32;
#if defined(_MSC_VER)
typedef   signed __int64        mal_int64;
typedef unsigned __int64        mal_uint64;
#else
typedef   signed long long int  mal_int64;
typedef unsigned long long int  mal_uint64;
#endif
#if defined(_WIN32)
#if defined(_WIN64)
typedef mal_uint64          mal_uintptr;
#else
typedef mal_uint32          mal_uintptr;
#endif
#elif defined(__GNUC__)
#if defined(__LP64__)
typedef mal_uint64          mal_uintptr;
#else
typedef mal_uint32          mal_uintptr;
#endif
#else
typedef mal_uint64          mal_uintptr;    // Fallback.
#endif
#else
#include <stdint.h>
typedef int8_t                      mal_int8;
typedef uint8_t                     mal_uint8;
typedef int16_t                     mal_int16;
typedef uint16_t                    mal_uint16;
typedef int32_t                     mal_int32;
typedef uint32_t                    mal_uint32;
typedef int64_t                     mal_int64;
typedef uint64_t                    mal_uint64;
typedef uintptr_t                   mal_uintptr;
#endif
typedef mal_uint8                   mal_bool8;
typedef mal_uint32                  mal_bool32;
#define MAL_TRUE                    1
#define MAL_FALSE                   0

typedef void* mal_handle;
typedef void* mal_ptr;
typedef void(*mal_proc)(void);

#if defined(_MSC_VER) && !defined(_WCHAR_T_DEFINED)
typedef mal_uint16 wchar_t;
#endif

// Define NULL for some compilers.
#ifndef NULL
#define NULL 0
#endif

#if defined(SIZE_MAX)
#define MAL_SIZE_MAX    SIZE_MAX
#else
#define MAL_SIZE_MAX    0xFFFFFFFF  /* When SIZE_MAX is not defined by the standard library just default to the maximum 32-bit unsigned integer. */
#endif


#ifdef _MSC_VER
#define MAL_INLINE __forceinline
#else
#ifdef __GNUC__
#define MAL_INLINE inline __attribute__((always_inline))
#else
#define MAL_INLINE inline
#endif
#endif

#ifdef _MSC_VER
#define MAL_ALIGN(alignment) __declspec(align(alignment))
#elif !defined(__DMC__)
#define MAL_ALIGN(alignment) __attribute__((aligned(alignment)))
#else
#define MAL_ALIGN(alignment)
#endif

#ifdef _MSC_VER
#define MAL_ALIGNED_STRUCT(alignment) MAL_ALIGN(alignment) struct
#else
#define MAL_ALIGNED_STRUCT(alignment) struct MAL_ALIGN(alignment)
#endif

// SIMD alignment in bytes. Currently set to 64 bytes in preparation for future AVX-512 optimizations.
#define MAL_SIMD_ALIGNMENT  64


// Logging levels
#define MAL_LOG_LEVEL_VERBOSE   4
#define MAL_LOG_LEVEL_INFO      3
#define MAL_LOG_LEVEL_WARNING   2
#define MAL_LOG_LEVEL_ERROR     1

#ifndef MAL_LOG_LEVEL
#define MAL_LOG_LEVEL           MAL_LOG_LEVEL_ERROR
#endif

typedef struct mal_context mal_context;
typedef struct mal_device mal_device;

typedef mal_uint8 mal_channel;
#define MAL_CHANNEL_NONE                                0
#define MAL_CHANNEL_MONO                                1
#define MAL_CHANNEL_FRONT_LEFT                          2
#define MAL_CHANNEL_FRONT_RIGHT                         3
#define MAL_CHANNEL_FRONT_CENTER                        4
#define MAL_CHANNEL_LFE                                 5
#define MAL_CHANNEL_BACK_LEFT                           6
#define MAL_CHANNEL_BACK_RIGHT                          7
#define MAL_CHANNEL_FRONT_LEFT_CENTER                   8
#define MAL_CHANNEL_FRONT_RIGHT_CENTER                  9
#define MAL_CHANNEL_BACK_CENTER                         10
#define MAL_CHANNEL_SIDE_LEFT                           11
#define MAL_CHANNEL_SIDE_RIGHT                          12
#define MAL_CHANNEL_TOP_CENTER                          13
#define MAL_CHANNEL_TOP_FRONT_LEFT                      14
#define MAL_CHANNEL_TOP_FRONT_CENTER                    15
#define MAL_CHANNEL_TOP_FRONT_RIGHT                     16
#define MAL_CHANNEL_TOP_BACK_LEFT                       17
#define MAL_CHANNEL_TOP_BACK_CENTER                     18
#define MAL_CHANNEL_TOP_BACK_RIGHT                      19
#define MAL_CHANNEL_AUX_0                               20
#define MAL_CHANNEL_AUX_1                               21
#define MAL_CHANNEL_AUX_2                               22
#define MAL_CHANNEL_AUX_3                               23
#define MAL_CHANNEL_AUX_4                               24
#define MAL_CHANNEL_AUX_5                               25
#define MAL_CHANNEL_AUX_6                               26
#define MAL_CHANNEL_AUX_7                               27
#define MAL_CHANNEL_AUX_8                               28
#define MAL_CHANNEL_AUX_9                               29
#define MAL_CHANNEL_AUX_10                              30
#define MAL_CHANNEL_AUX_11                              31
#define MAL_CHANNEL_AUX_12                              32
#define MAL_CHANNEL_AUX_13                              33
#define MAL_CHANNEL_AUX_14                              34
#define MAL_CHANNEL_AUX_15                              35
#define MAL_CHANNEL_AUX_16                              36
#define MAL_CHANNEL_AUX_17                              37
#define MAL_CHANNEL_AUX_18                              38
#define MAL_CHANNEL_AUX_19                              39
#define MAL_CHANNEL_AUX_20                              40
#define MAL_CHANNEL_AUX_21                              41
#define MAL_CHANNEL_AUX_22                              42
#define MAL_CHANNEL_AUX_23                              43
#define MAL_CHANNEL_AUX_24                              44
#define MAL_CHANNEL_AUX_25                              45
#define MAL_CHANNEL_AUX_26                              46
#define MAL_CHANNEL_AUX_27                              47
#define MAL_CHANNEL_AUX_28                              48
#define MAL_CHANNEL_AUX_29                              49
#define MAL_CHANNEL_AUX_30                              50
#define MAL_CHANNEL_AUX_31                              51
#define MAL_CHANNEL_LEFT                                MAL_CHANNEL_FRONT_LEFT
#define MAL_CHANNEL_RIGHT                               MAL_CHANNEL_FRONT_RIGHT
#define MAL_CHANNEL_POSITION_COUNT                      MAL_CHANNEL_AUX_31 + 1

typedef int mal_result;
#define MAL_SUCCESS                                      0
#define MAL_ERROR                                       -1      // A generic error.
#define MAL_INVALID_ARGS                                -2
#define MAL_INVALID_OPERATION                           -3
#define MAL_OUT_OF_MEMORY                               -4
#define MAL_FORMAT_NOT_SUPPORTED                        -5
#define MAL_NO_BACKEND                                  -6
#define MAL_NO_DEVICE                                   -7
#define MAL_API_NOT_FOUND                               -8
#define MAL_DEVICE_BUSY                                 -9
#define MAL_DEVICE_NOT_INITIALIZED                      -10
#define MAL_DEVICE_NOT_STARTED                          -11
#define MAL_DEVICE_NOT_STOPPED                          -12
#define MAL_DEVICE_ALREADY_STARTED                      -13
#define MAL_DEVICE_ALREADY_STARTING                     -14
#define MAL_DEVICE_ALREADY_STOPPED                      -15
#define MAL_DEVICE_ALREADY_STOPPING                     -16
#define MAL_FAILED_TO_MAP_DEVICE_BUFFER                 -17
#define MAL_FAILED_TO_UNMAP_DEVICE_BUFFER               -18
#define MAL_FAILED_TO_INIT_BACKEND                      -19
#define MAL_FAILED_TO_READ_DATA_FROM_CLIENT             -20
#define MAL_FAILED_TO_READ_DATA_FROM_DEVICE             -21
#define MAL_FAILED_TO_SEND_DATA_TO_CLIENT               -22
#define MAL_FAILED_TO_SEND_DATA_TO_DEVICE               -23
#define MAL_FAILED_TO_OPEN_BACKEND_DEVICE               -24
#define MAL_FAILED_TO_START_BACKEND_DEVICE              -25
#define MAL_FAILED_TO_STOP_BACKEND_DEVICE               -26
#define MAL_FAILED_TO_CONFIGURE_BACKEND_DEVICE          -27
#define MAL_FAILED_TO_CREATE_MUTEX                      -28
#define MAL_FAILED_TO_CREATE_EVENT                      -29
#define MAL_FAILED_TO_CREATE_THREAD                     -30
#define MAL_INVALID_DEVICE_CONFIG                       -31
#define MAL_ACCESS_DENIED                               -32
#define MAL_TOO_LARGE                                   -33
#define MAL_DEVICE_UNAVAILABLE                          -34
#define MAL_TIMEOUT                                     -35

// Standard sample rates.
#define MAL_SAMPLE_RATE_8000                            8000
#define MAL_SAMPLE_RATE_11025                           11025
#define MAL_SAMPLE_RATE_16000                           16000
#define MAL_SAMPLE_RATE_22050                           22050
#define MAL_SAMPLE_RATE_24000                           24000
#define MAL_SAMPLE_RATE_32000                           32000
#define MAL_SAMPLE_RATE_44100                           44100
#define MAL_SAMPLE_RATE_48000                           48000
#define MAL_SAMPLE_RATE_88200                           88200
#define MAL_SAMPLE_RATE_96000                           96000
#define MAL_SAMPLE_RATE_176400                          176400
#define MAL_SAMPLE_RATE_192000                          192000
#define MAL_SAMPLE_RATE_352800                          352800
#define MAL_SAMPLE_RATE_384000                          384000

#define MAL_MIN_PCM_SAMPLE_SIZE_IN_BYTES                1   // For simplicity, mini_al does not support PCM samples that are not byte aligned.
#define MAL_MAX_PCM_SAMPLE_SIZE_IN_BYTES                8
#define MAL_MIN_CHANNELS                                1
#define MAL_MAX_CHANNELS                                32
#define MAL_MIN_SAMPLE_RATE                             MAL_SAMPLE_RATE_8000
#define MAL_MAX_SAMPLE_RATE                             MAL_SAMPLE_RATE_384000
#define MAL_SRC_SINC_MIN_WINDOW_WIDTH                   2
#define MAL_SRC_SINC_MAX_WINDOW_WIDTH                   32
#define MAL_SRC_SINC_DEFAULT_WINDOW_WIDTH               32
#define MAL_SRC_SINC_LOOKUP_TABLE_RESOLUTION            8
#define MAL_SRC_INPUT_BUFFER_SIZE_IN_SAMPLES            256

typedef enum
{
	mal_stream_format_pcm = 0,
} mal_stream_format;

typedef enum
{
	mal_stream_layout_interleaved = 0,
	mal_stream_layout_deinterleaved
} mal_stream_layout;

typedef enum
{
	mal_dither_mode_none = 0,
	mal_dither_mode_rectangle,
	mal_dither_mode_triangle
} mal_dither_mode;

typedef enum
{
	// I like to keep these explicitly defined because they're used as a key into a lookup table. When items are
	// added to this, make sure there are no gaps and that they're added to the lookup table in mal_get_bytes_per_sample().
	mal_format_unknown = 0,     // Mainly used for indicating an error, but also used as the default for the output format for decoders.
	mal_format_u8 = 1,
	mal_format_s16 = 2,     // Seems to be the most widely supported format.
	mal_format_s24 = 3,     // Tightly packed. 3 bytes per sample.
	mal_format_s32 = 4,
	mal_format_f32 = 5,
	mal_format_count
} mal_format;

typedef enum
{
	mal_channel_mix_mode_rectangular = 0,   // Simple averaging based on the plane(s) the channel is sitting on.
	mal_channel_mix_mode_simple,            // Drop excess channels; zeroed out extra channels.
	mal_channel_mix_mode_custom_weights,    // Use custom weights specified in mal_channel_router_config.
	mal_channel_mix_mode_planar_blend = mal_channel_mix_mode_rectangular,
	mal_channel_mix_mode_default = mal_channel_mix_mode_planar_blend
} mal_channel_mix_mode;

typedef enum
{
	mal_standard_channel_map_microsoft,
	mal_standard_channel_map_alsa,
	mal_standard_channel_map_rfc3551,   // Based off AIFF.
	mal_standard_channel_map_flac,
	mal_standard_channel_map_vorbis,
	mal_standard_channel_map_sound4,    // FreeBSD's sound(4).
	mal_standard_channel_map_sndio,     // www.sndio.org/tips.html
	mal_standard_channel_map_default = mal_standard_channel_map_microsoft
} mal_standard_channel_map;

typedef enum
{
	mal_performance_profile_low_latency = 0,
	mal_performance_profile_conservative
} mal_performance_profile;


typedef struct mal_format_converter mal_format_converter;
typedef mal_uint32(*mal_format_converter_read_proc)              (mal_format_converter* pConverter, mal_uint32 frameCount, void* pFramesOut, void* pUserData);
typedef mal_uint32(*mal_format_converter_read_deinterleaved_proc)(mal_format_converter* pConverter, mal_uint32 frameCount, void** ppSamplesOut, void* pUserData);

typedef struct
{
	mal_format formatIn;
	mal_format formatOut;
	mal_uint32 channels;
	mal_stream_format streamFormatIn;
	mal_stream_format streamFormatOut;
	mal_dither_mode ditherMode;
	mal_bool32 noSSE2 : 1;
	mal_bool32 noAVX2 : 1;
	mal_bool32 noAVX512 : 1;
	mal_bool32 noNEON : 1;
	mal_format_converter_read_proc onRead;
	mal_format_converter_read_deinterleaved_proc onReadDeinterleaved;
	void* pUserData;
} mal_format_converter_config;

struct mal_format_converter
{
	mal_format_converter_config config;
	mal_bool32 useSSE2 : 1;
	mal_bool32 useAVX2 : 1;
	mal_bool32 useAVX512 : 1;
	mal_bool32 useNEON : 1;
	void(*onConvertPCM)(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode);
	void(*onInterleavePCM)(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels);
	void(*onDeinterleavePCM)(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels);
};



typedef struct mal_channel_router mal_channel_router;
typedef mal_uint32(*mal_channel_router_read_deinterleaved_proc)(mal_channel_router* pRouter, mal_uint32 frameCount, void** ppSamplesOut, void* pUserData);

typedef struct
{
	mal_uint32 channelsIn;
	mal_uint32 channelsOut;
	mal_channel channelMapIn[MAL_MAX_CHANNELS];
	mal_channel channelMapOut[MAL_MAX_CHANNELS];
	mal_channel_mix_mode mixingMode;
	float weights[MAL_MAX_CHANNELS][MAL_MAX_CHANNELS];  // [in][out]. Only used when mixingMode is set to mal_channel_mix_mode_custom_weights.
	mal_bool32 noSSE2 : 1;
	mal_bool32 noAVX2 : 1;
	mal_bool32 noAVX512 : 1;
	mal_bool32 noNEON : 1;
	mal_channel_router_read_deinterleaved_proc onReadDeinterleaved;
	void* pUserData;
} mal_channel_router_config;

struct mal_channel_router
{
	mal_channel_router_config config;
	mal_bool32 isPassthrough : 1;
	mal_bool32 isSimpleShuffle : 1;
	mal_bool32 useSSE2 : 1;
	mal_bool32 useAVX2 : 1;
	mal_bool32 useAVX512 : 1;
	mal_bool32 useNEON : 1;
	mal_uint8 shuffleTable[MAL_MAX_CHANNELS];
};



typedef struct mal_src mal_src;
//typedef mal_uint32 (* mal_src_read_proc)(mal_src* pSRC, mal_uint32 frameCount, void* pFramesOut, void* pUserData); // Returns the number of frames that were read.
typedef mal_uint32(*mal_src_read_deinterleaved_proc)(mal_src* pSRC, mal_uint32 frameCount, void** ppSamplesOut, void* pUserData); // Returns the number of frames that were read.

typedef enum
{
	mal_src_algorithm_sinc = 0,
	mal_src_algorithm_linear,
	mal_src_algorithm_none,
	mal_src_algorithm_default = mal_src_algorithm_sinc
} mal_src_algorithm;

typedef enum
{
	mal_src_sinc_window_function_hann = 0,
	mal_src_sinc_window_function_rectangular,
	mal_src_sinc_window_function_default = mal_src_sinc_window_function_hann
} mal_src_sinc_window_function;

typedef struct
{
	mal_src_sinc_window_function windowFunction;
	mal_uint32 windowWidth;
} mal_src_config_sinc;

typedef struct
{
	mal_uint32 sampleRateIn;
	mal_uint32 sampleRateOut;
	mal_uint32 channels;
	mal_src_algorithm algorithm;
	mal_bool32 neverConsumeEndOfInput : 1;
	mal_bool32 noSSE2 : 1;
	mal_bool32 noAVX2 : 1;
	mal_bool32 noAVX512 : 1;
	mal_bool32 noNEON : 1;
	mal_src_read_deinterleaved_proc onReadDeinterleaved;
	void* pUserData;
	union
	{
		mal_src_config_sinc sinc;
	};
} mal_src_config;

MAL_ALIGNED_STRUCT(MAL_SIMD_ALIGNMENT) mal_src
{
	union
	{
		struct
		{
			MAL_ALIGN(MAL_SIMD_ALIGNMENT) float input[MAL_MAX_CHANNELS][MAL_SRC_INPUT_BUFFER_SIZE_IN_SAMPLES];
			float timeIn;
			mal_uint32 leftoverFrames;
		} linear;

		struct
		{
			MAL_ALIGN(MAL_SIMD_ALIGNMENT) float input[MAL_MAX_CHANNELS][MAL_SRC_SINC_MAX_WINDOW_WIDTH * 2 + MAL_SRC_INPUT_BUFFER_SIZE_IN_SAMPLES];
			float timeIn;
			mal_uint32 inputFrameCount;     // The number of frames sitting in the input buffer, not including the first half of the window.
			mal_uint32 windowPosInSamples;  // An offset of <input>.
			float table[MAL_SRC_SINC_MAX_WINDOW_WIDTH * 1 * MAL_SRC_SINC_LOOKUP_TABLE_RESOLUTION]; // Precomputed lookup table. The +1 is used to avoid the need for an overflow check.
		} sinc;
	};

	mal_src_config config;
	mal_bool32 isEndOfInputLoaded : 1;
	mal_bool32 useSSE2 : 1;
	mal_bool32 useAVX2 : 1;
	mal_bool32 useAVX512 : 1;
	mal_bool32 useNEON : 1;
};

typedef struct mal_dsp mal_dsp;
typedef mal_uint32(*mal_dsp_read_proc)(mal_dsp* pDSP, mal_uint32 frameCount, void* pSamplesOut, void* pUserData);

typedef struct
{
	mal_format formatIn;
	mal_uint32 channelsIn;
	mal_uint32 sampleRateIn;
	mal_channel channelMapIn[MAL_MAX_CHANNELS];
	mal_format formatOut;
	mal_uint32 channelsOut;
	mal_uint32 sampleRateOut;
	mal_channel channelMapOut[MAL_MAX_CHANNELS];
	mal_channel_mix_mode channelMixMode;
	mal_dither_mode ditherMode;
	mal_src_algorithm srcAlgorithm;
	mal_bool32 allowDynamicSampleRate;
	mal_bool32 neverConsumeEndOfInput : 1;  // <-- For SRC.
	mal_bool32 noSSE2 : 1;
	mal_bool32 noAVX2 : 1;
	mal_bool32 noAVX512 : 1;
	mal_bool32 noNEON : 1;
	mal_dsp_read_proc onRead;
	void* pUserData;
	union
	{
		mal_src_config_sinc sinc;
	};
} mal_dsp_config;

MAL_ALIGNED_STRUCT(MAL_SIMD_ALIGNMENT) mal_dsp
{
	mal_dsp_read_proc onRead;
	void* pUserData;
	mal_format_converter formatConverterIn;             // For converting data to f32 in preparation for further processing.
	mal_format_converter formatConverterOut;            // For converting data to the requested output format. Used as the final step in the processing pipeline.
	mal_channel_router channelRouter;                   // For channel conversion.
	mal_src src;                                        // For sample rate conversion.
	mal_bool32 isDynamicSampleRateAllowed : 1;      // mal_dsp_set_input_sample_rate() and mal_dsp_set_output_sample_rate() will fail if this is set to false.
	mal_bool32 isPreFormatConversionRequired : 1;
	mal_bool32 isPostFormatConversionRequired : 1;
	mal_bool32 isChannelRoutingRequired : 1;
	mal_bool32 isSRCRequired : 1;
	mal_bool32 isChannelRoutingAtStart : 1;
	mal_bool32 isPassthrough : 1;      // <-- Will be set to true when the DSP pipeline is an optimized passthrough.
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DATA CONVERSION
// ===============
//
// This section contains the APIs for data conversion. You will find everything here for channel mapping, sample format conversion, resampling, etc.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Channel Maps
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Helper for retrieving a standard channel map.
void mal_get_standard_channel_map(mal_standard_channel_map standardChannelMap, mal_uint32 channels, mal_channel channelMap[MAL_MAX_CHANNELS]);

// Copies a channel map.
void mal_channel_map_copy(mal_channel* pOut, const mal_channel* pIn, mal_uint32 channels);


// Determines whether or not a channel map is valid.
//
// A blank channel map is valid (all channels set to MAL_CHANNEL_NONE). The way a blank channel map is handled is context specific, but
// is usually treated as a passthrough.
//
// Invalid channel maps:
//   - A channel map with no channels
//   - A channel map with more than one channel and a mono channel
mal_bool32 mal_channel_map_valid(mal_uint32 channels, const mal_channel channelMap[MAL_MAX_CHANNELS]);

// Helper for comparing two channel maps for equality.
//
// This assumes the channel count is the same between the two.
mal_bool32 mal_channel_map_equal(mal_uint32 channels, const mal_channel channelMapA[MAL_MAX_CHANNELS], const mal_channel channelMapB[MAL_MAX_CHANNELS]);

// Helper for determining if a channel map is blank (all channels set to MAL_CHANNEL_NONE).
mal_bool32 mal_channel_map_blank(mal_uint32 channels, const mal_channel channelMap[MAL_MAX_CHANNELS]);

// Helper for determining whether or not a channel is present in the given channel map.
mal_bool32 mal_channel_map_contains_channel_position(mal_uint32 channels, const mal_channel channelMap[MAL_MAX_CHANNELS], mal_channel channelPosition);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Format Conversion
// =================
// The format converter serves two purposes:
//   1) Conversion between data formats (u8 to f32, etc.)
//   2) Interleaving and deinterleaving
//
// When initializing a converter, you specify the input and output formats (u8, s16, etc.) and read callbacks. There are two read callbacks - one for
// interleaved input data (onRead) and another for deinterleaved input data (onReadDeinterleaved). You implement whichever is most convenient for you. You
// can implement both, but it's not recommended as it just introduces unnecessary complexity.
//
// To read data as interleaved samples, use mal_format_converter_read(). Otherwise use mal_format_converter_read_deinterleaved().
//
// Dithering
// ---------
// The format converter also supports dithering. Dithering can be set using ditherMode variable in the config, like so.
//
//   pConfig->ditherMode = mal_dither_mode_rectangle;
//
// The different dithering modes include the following, in order of efficiency:
//   - None:      mal_dither_mode_none
//   - Rectangle: mal_dither_mode_rectangle
//   - Triangle:  mal_dither_mode_triangle
//
// Note that even if the dither mode is set to something other than mal_dither_mode_none, it will be ignored for conversions where dithering is not needed.
// Dithering is available for the following conversions:
//   - s16 -> u8
//   - s24 -> u8
//   - s32 -> u8
//   - f32 -> u8
//   - s24 -> s16
//   - s32 -> s16
//   - f32 -> s16
//
// Note that it is not an error to pass something other than mal_dither_mode_none for conversions where dither is not used. It will just be ignored.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initializes a format converter.
mal_result mal_format_converter_init(const mal_format_converter_config* pConfig, mal_format_converter* pConverter);

// Reads data from the format converter as interleaved channels.
mal_uint64 mal_format_converter_read(mal_format_converter* pConverter, mal_uint64 frameCount, void* pFramesOut, void* pUserData);

// Reads data from the format converter as deinterleaved channels.
mal_uint64 mal_format_converter_read_deinterleaved(mal_format_converter* pConverter, mal_uint64 frameCount, void** ppSamplesOut, void* pUserData);


// Helper for initializing a format converter config.
mal_format_converter_config mal_format_converter_config_init_new(void);
mal_format_converter_config mal_format_converter_config_init(mal_format formatIn, mal_format formatOut, mal_uint32 channels, mal_format_converter_read_proc onRead, void* pUserData);
mal_format_converter_config mal_format_converter_config_init_deinterleaved(mal_format formatIn, mal_format formatOut, mal_uint32 channels, mal_format_converter_read_deinterleaved_proc onReadDeinterleaved, void* pUserData);



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Channel Routing
// ===============
// There are two main things you can do with the channel router:
//   1) Rearrange channels
//   2) Convert from one channel count to another
//
// Channel Rearrangement
// ---------------------
// A simple example of channel rearrangement may be swapping the left and right channels in a stereo stream. To do this you just pass in the same channel
// count for both the input and output with channel maps that contain the same channels (in a different order).
//
// Channel Conversion
// ------------------
// The channel router can also convert from one channel count to another, such as converting a 5.1 stream to stero. When changing the channel count, the
// router will first perform a 1:1 mapping of channel positions that are present in both the input and output channel maps. The second thing it will do
// is distribute the input mono channel (if any) across all output channels, excluding any None and LFE channels. If there is an output mono channel, all
// input channels will be averaged, excluding any None and LFE channels.
//
// The last case to consider is when a channel position in the input channel map is not present in the output channel map, and vice versa. In this case the
// channel router will perform a blend of other related channels to produce an audible channel. There are several blending modes.
//   1) Simple
//      Unmatched channels are silenced.
//   2) Planar Blending
//      Channels are blended based on a set of planes that each speaker emits audio from.
//
// Rectangular / Planar Blending
// -----------------------------
// In this mode, channel positions are associated with a set of planes where the channel conceptually emits audio from. An example is the front/left speaker.
// This speaker is positioned to the front of the listener, so you can think of it as emitting audio from the front plane. It is also positioned to the left
// of the listener so you can think of it as also emitting audio from the left plane. Now consider the (unrealistic) situation where the input channel map
// contains only the front/left channel position, but the output channel map contains both the front/left and front/center channel. When deciding on the audio
// data to send to the front/center speaker (which has no 1:1 mapping with an input channel) we need to use some logic based on our available input channel
// positions.
//
// As mentioned earlier, our front/left speaker is, conceptually speaking, emitting audio from the front _and_ the left planes. Similarly, the front/center
// speaker is emitting audio from _only_ the front plane. What these two channels have in common is that they are both emitting audio from the front plane.
// Thus, it makes sense that the front/center speaker should receive some contribution from the front/left channel. How much contribution depends on their
// planar relationship (thus the name of this blending technique).
//
// Because the front/left channel is emitting audio from two planes (front and left), you can think of it as though it's willing to dedicate 50% of it's total
// volume to each of it's planes (a channel position emitting from 1 plane would be willing to given 100% of it's total volume to that plane, and a channel
// position emitting from 3 planes would be willing to given 33% of it's total volume to each plane). Similarly, the front/center speaker is emitting audio
// from only one plane so you can think of it as though it's willing to _take_ 100% of it's volume from front plane emissions. Now, since the front/left
// channel is willing to _give_ 50% of it's total volume to the front plane, and the front/center speaker is willing to _take_ 100% of it's total volume
// from the front, you can imagine that 50% of the front/left speaker will be given to the front/center speaker.
//
// Usage
// -----
// To use the channel router you need to specify three things:
//   1) The input channel count and channel map
//   2) The output channel count and channel map
//   3) The mixing mode to use in the case where a 1:1 mapping is unavailable
//
// Note that input and output data is always deinterleaved 32-bit floating point.
//
// Initialize the channel router with mal_channel_router_init(). You will need to pass in a config object which specifies the input and output configuration,
// mixing mode and a callback for sending data to the router. This callback will be called when input data needs to be sent to the router for processing. Note
// that the mixing mode is only used when a 1:1 mapping is unavailable. This includes the custom weights mode.
//
// Read data from the channel router with mal_channel_router_read_deinterleaved(). Output data is always 32-bit floating point.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initializes a channel router where it is assumed that the input data is non-interleaved.
mal_result mal_channel_router_init(const mal_channel_router_config* pConfig, mal_channel_router* pRouter);

// Reads data from the channel router as deinterleaved channels.
mal_uint64 mal_channel_router_read_deinterleaved(mal_channel_router* pRouter, mal_uint64 frameCount, void** ppSamplesOut, void* pUserData);

// Helper for initializing a channel router config.
mal_channel_router_config mal_channel_router_config_init(mal_uint32 channelsIn, const mal_channel channelMapIn[MAL_MAX_CHANNELS], mal_uint32 channelsOut, const mal_channel channelMapOut[MAL_MAX_CHANNELS], mal_channel_mix_mode mixingMode, mal_channel_router_read_deinterleaved_proc onRead, void* pUserData);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sample Rate Conversion
// ======================
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initializes a sample rate conversion object.
mal_result mal_src_init(const mal_src_config* pConfig, mal_src* pSRC);

// Dynamically adjusts the input sample rate.
//
// DEPRECATED. Use mal_src_set_sample_rate() instead.
mal_result mal_src_set_input_sample_rate(mal_src* pSRC, mal_uint32 sampleRateIn);

// Dynamically adjusts the output sample rate.
//
// This is useful for dynamically adjust pitch. Keep in mind, however, that this will speed up or slow down the sound. If this
// is not acceptable you will need to use your own algorithm.
//
// DEPRECATED. Use mal_src_set_sample_rate() instead.
mal_result mal_src_set_output_sample_rate(mal_src* pSRC, mal_uint32 sampleRateOut);

// Dynamically adjusts the sample rate.
//
// This is useful for dynamically adjust pitch. Keep in mind, however, that this will speed up or slow down the sound. If this
// is not acceptable you will need to use your own algorithm.
mal_result mal_src_set_sample_rate(mal_src* pSRC, mal_uint32 sampleRateIn, mal_uint32 sampleRateOut);

// Reads a number of frames.
//
// Returns the number of frames actually read.
mal_uint64 mal_src_read_deinterleaved(mal_src* pSRC, mal_uint64 frameCount, void** ppSamplesOut, void* pUserData);


// Helper for creating a sample rate conversion config.
mal_src_config mal_src_config_init_new(void);
mal_src_config mal_src_config_init(mal_uint32 sampleRateIn, mal_uint32 sampleRateOut, mal_uint32 channels, mal_src_read_deinterleaved_proc onReadDeinterleaved, void* pUserData);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DSP
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initializes a DSP object.
mal_result mal_dsp_init(const mal_dsp_config* pConfig, mal_dsp* pDSP);

// Dynamically adjusts the input sample rate.
//
// This will fail is the DSP was not initialized with allowDynamicSampleRate.
//
// DEPRECATED. Use mal_dsp_set_sample_rate() instead.
mal_result mal_dsp_set_input_sample_rate(mal_dsp* pDSP, mal_uint32 sampleRateOut);

// Dynamically adjusts the output sample rate.
//
// This is useful for dynamically adjust pitch. Keep in mind, however, that this will speed up or slow down the sound. If this
// is not acceptable you will need to use your own algorithm.
//
// This will fail is the DSP was not initialized with allowDynamicSampleRate.
//
// DEPRECATED. Use mal_dsp_set_sample_rate() instead.
mal_result mal_dsp_set_output_sample_rate(mal_dsp* pDSP, mal_uint32 sampleRateOut);

// Dynamically adjusts the output sample rate.
//
// This is useful for dynamically adjust pitch. Keep in mind, however, that this will speed up or slow down the sound. If this
// is not acceptable you will need to use your own algorithm.
//
// This will fail is the DSP was not initialized with allowDynamicSampleRate.
mal_result mal_dsp_set_sample_rate(mal_dsp* pDSP, mal_uint32 sampleRateIn, mal_uint32 sampleRateOut);


// Reads a number of frames and runs them through the DSP processor.
mal_uint64 mal_dsp_read(mal_dsp* pDSP, mal_uint64 frameCount, void* pFramesOut, void* pUserData);

// Helper for initializing a mal_dsp_config object.
mal_dsp_config mal_dsp_config_init_new(void);
mal_dsp_config mal_dsp_config_init(mal_format formatIn, mal_uint32 channelsIn, mal_uint32 sampleRateIn, mal_format formatOut, mal_uint32 channelsOut, mal_uint32 sampleRateOut, mal_dsp_read_proc onRead, void* pUserData);
mal_dsp_config mal_dsp_config_init_ex(mal_format formatIn, mal_uint32 channelsIn, mal_uint32 sampleRateIn, mal_channel channelMapIn[MAL_MAX_CHANNELS], mal_format formatOut, mal_uint32 channelsOut, mal_uint32 sampleRateOut, mal_channel channelMapOut[MAL_MAX_CHANNELS], mal_dsp_read_proc onRead, void* pUserData);


// High-level helper for doing a full format conversion in one go. Returns the number of output frames. Call this with pOut set to NULL to
// determine the required size of the output buffer.
//
// A return value of 0 indicates an error.
//
// This function is useful for one-off bulk conversions, but if you're streaming data you should use the DSP APIs instead.
mal_uint64 mal_convert_frames(void* pOut, mal_format formatOut, mal_uint32 channelsOut, mal_uint32 sampleRateOut, const void* pIn, mal_format formatIn, mal_uint32 channelsIn, mal_uint32 sampleRateIn, mal_uint64 frameCountIn);
mal_uint64 mal_convert_frames_ex(void* pOut, mal_format formatOut, mal_uint32 channelsOut, mal_uint32 sampleRateOut, mal_channel channelMapOut[MAL_MAX_CHANNELS], const void* pIn, mal_format formatIn, mal_uint32 channelsIn, mal_uint32 sampleRateIn, mal_channel channelMapIn[MAL_MAX_CHANNELS], mal_uint64 frameCountIn);



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Miscellaneous Helpers
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// malloc(). Calls MAL_MALLOC().
void* mal_malloc(size_t sz);

// realloc(). Calls MAL_REALLOC().
void* mal_realloc(void* p, size_t sz);

// free(). Calls MAL_FREE().
void mal_free(void* p);

// Performs an aligned malloc, with the assumption that the alignment is a power of 2.
void* mal_aligned_malloc(size_t sz, size_t alignment);

// Free's an aligned malloc'd buffer.
void mal_aligned_free(void* p);

// Retrieves a friendly name for a format.
const char* mal_get_format_name(mal_format format);

// Blends two frames in floating point format.
void mal_blend_f32(float* pOut, float* pInA, float* pInB, float factor, mal_uint32 channels);

// Retrieves the size of a sample in bytes for the given format.
//
// This API is efficient and is implemented using a lookup table.
//
// Thread Safety: SAFE
//   This is API is pure.
mal_uint32 mal_get_bytes_per_sample(mal_format format);
static MAL_INLINE mal_uint32 mal_get_bytes_per_frame(mal_format format, mal_uint32 channels) { return mal_get_bytes_per_sample(format) * channels; }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Format Conversion
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void mal_pcm_u8_to_s16(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_u8_to_s24(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_u8_to_s32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_u8_to_f32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s16_to_u8(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s16_to_s24(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s16_to_s32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s16_to_f32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s24_to_u8(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s24_to_s16(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s24_to_s32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s24_to_f32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s32_to_u8(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s32_to_s16(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s32_to_s24(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s32_to_f32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_f32_to_u8(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_f32_to_s16(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_f32_to_s24(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_f32_to_s32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_convert(void* pOut, mal_format formatOut, const void* pIn, mal_format formatIn, mal_uint64 sampleCount, mal_dither_mode ditherMode);

// Deinterleaves an interleaved buffer.
void mal_deinterleave_pcm_frames(mal_format format, mal_uint32 channels, mal_uint64 frameCount, const void* pInterleavedPCMFrames, void** ppDeinterleavedPCMFrames);

// Interleaves a group of deinterleaved buffers.
void mal_interleave_pcm_frames(mal_format format, mal_uint32 channels, mal_uint64 frameCount, const void** ppDeinterleavedPCMFrames, void* pInterleavedPCMFrames);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DEVICE I/O
// ==========
//
// This section contains the APIs for device playback and capture. Here is where you'll find mal_device_init(), etc.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef MAL_NO_DEVICE_IO
// Some backends are only supported on certain platforms.
#if defined(MAL_WIN32)
#define MAL_SUPPORT_WASAPI
#if defined(MAL_WIN32_DESKTOP)  // DirectSound and WinMM backends are only supported on desktop's.
#define MAL_SUPPORT_DSOUND
#define MAL_SUPPORT_WINMM
#define MAL_SUPPORT_JACK    // JACK is technically supported on Windows, but I don't know how many people use it in practice...
#endif
#endif
#if defined(MAL_UNIX)
#if defined(MAL_LINUX)
#if !defined(MAL_ANDROID)   // ALSA is not supported on Android.
#define MAL_SUPPORT_ALSA
#endif
#endif
#if !defined(MAL_BSD) && !defined(MAL_ANDROID) && !defined(MAL_EMSCRIPTEN)
#define MAL_SUPPORT_PULSEAUDIO
#define MAL_SUPPORT_JACK
#endif
#if defined(MAL_ANDROID)
#define MAL_SUPPORT_OPENSL
#endif
#if defined(__OpenBSD__)    // <-- Change this to "#if defined(MAL_BSD)" to enable sndio on all BSD flavors.
#define MAL_SUPPORT_SNDIO   // sndio is only supported on OpenBSD for now. May be expanded later if there's demand.
#endif
#if defined(__NetBSD__) || defined(__OpenBSD__)
#define MAL_SUPPORT_AUDIO4  // Only support audio(4) on platforms with known support.
#endif
#if defined(__FreeBSD__) || defined(__DragonFly__)
#define MAL_SUPPORT_OSS     // Only support OSS on specific platforms with known support.
#endif
#endif
#if defined(MAL_APPLE)
#define MAL_SUPPORT_COREAUDIO
#endif

#define MAL_SUPPORT_SDL     // All platforms support SDL.

// Explicitly disable OpenAL and Null backends for Emscripten because they both use a background thread which is not properly supported right now.
#if !defined(MAL_EMSCRIPTEN)
#define MAL_SUPPORT_OPENAL
#define MAL_SUPPORT_NULL    // All platforms support the null backend.
#endif


#if !defined(MAL_NO_WASAPI) && defined(MAL_SUPPORT_WASAPI)
#define MAL_ENABLE_WASAPI
#endif
#if !defined(MAL_NO_DSOUND) && defined(MAL_SUPPORT_DSOUND)
#define MAL_ENABLE_DSOUND
#endif
#if !defined(MAL_NO_WINMM) && defined(MAL_SUPPORT_WINMM)
#define MAL_ENABLE_WINMM
#endif
#if !defined(MAL_NO_ALSA) && defined(MAL_SUPPORT_ALSA)
#define MAL_ENABLE_ALSA
#endif
#if !defined(MAL_NO_PULSEAUDIO) && defined(MAL_SUPPORT_PULSEAUDIO)
#define MAL_ENABLE_PULSEAUDIO
#endif
#if !defined(MAL_NO_JACK) && defined(MAL_SUPPORT_JACK)
#define MAL_ENABLE_JACK
#endif
#if !defined(MAL_NO_COREAUDIO) && defined(MAL_SUPPORT_COREAUDIO)
#define MAL_ENABLE_COREAUDIO
#endif
#if !defined(MAL_NO_SNDIO) && defined(MAL_SUPPORT_SNDIO)
#define MAL_ENABLE_SNDIO
#endif
#if !defined(MAL_NO_AUDIO4) && defined(MAL_SUPPORT_AUDIO4)
#define MAL_ENABLE_AUDIO4
#endif
#if !defined(MAL_NO_OSS) && defined(MAL_SUPPORT_OSS)
#define MAL_ENABLE_OSS
#endif
#if !defined(MAL_NO_OPENSL) && defined(MAL_SUPPORT_OPENSL)
#define MAL_ENABLE_OPENSL
#endif
#if !defined(MAL_NO_OPENAL) && defined(MAL_SUPPORT_OPENAL)
#define MAL_ENABLE_OPENAL
#endif
#if !defined(MAL_NO_SDL) && defined(MAL_SUPPORT_SDL)
#define MAL_ENABLE_SDL
#endif
#if !defined(MAL_NO_NULL) && defined(MAL_SUPPORT_NULL)
#define MAL_ENABLE_NULL
#endif

#ifdef MAL_SUPPORT_WASAPI
// We need a IMMNotificationClient object for WASAPI. 
typedef struct
{
	void* lpVtbl;
	mal_uint32 counter;
	mal_device* pDevice;
} mal_IMMNotificationClient;
#endif


typedef enum
{
	mal_backend_null,
	mal_backend_wasapi,
	mal_backend_dsound,
	mal_backend_winmm,
	mal_backend_alsa,
	mal_backend_pulseaudio,
	mal_backend_jack,
	mal_backend_coreaudio,
	mal_backend_sndio,
	mal_backend_audio4,
	mal_backend_oss,
	mal_backend_opensl,
	mal_backend_openal,
	mal_backend_sdl
} mal_backend;

// Thread priorties should be ordered such that the default priority of the worker thread is 0.
typedef enum
{
	mal_thread_priority_idle = -5,
	mal_thread_priority_lowest = -4,
	mal_thread_priority_low = -3,
	mal_thread_priority_normal = -2,
	mal_thread_priority_high = -1,
	mal_thread_priority_highest = 0,
	mal_thread_priority_realtime = 1,
	mal_thread_priority_default = 0
} mal_thread_priority;

typedef struct
{
	mal_context* pContext;

	union
	{
#ifdef MAL_WIN32
		struct
		{
			/*HANDLE*/ mal_handle hThread;
		} win32;
#endif
#ifdef MAL_POSIX
		struct
		{
			pthread_t thread;
		} posix;
#endif

		int _unused;
	};
} mal_thread;

typedef struct
{
	mal_context* pContext;

	union
	{
#ifdef MAL_WIN32
		struct
		{
			/*HANDLE*/ mal_handle hMutex;
		} win32;
#endif
#ifdef MAL_POSIX
		struct
		{
			pthread_mutex_t mutex;
		} posix;
#endif

		int _unused;
	};
} mal_mutex;

typedef struct
{
	mal_context* pContext;

	union
	{
#ifdef MAL_WIN32
		struct
		{
			/*HANDLE*/ mal_handle hEvent;
		} win32;
#endif
#ifdef MAL_POSIX
		struct
		{
			pthread_mutex_t mutex;
			pthread_cond_t condition;
			mal_uint32 value;
		} posix;
#endif

		int _unused;
	};
} mal_event;


#define MAL_MAX_PERIODS_DSOUND                          4
#define MAL_MAX_PERIODS_OPENAL                          4

typedef void(*mal_log_proc) (mal_context* pContext, mal_device* pDevice, const char* message);
typedef void(*mal_recv_proc)(mal_device* pDevice, mal_uint32 frameCount, const void* pSamples);
typedef mal_uint32(*mal_send_proc)(mal_device* pDevice, mal_uint32 frameCount, void* pSamples);
typedef void(*mal_stop_proc)(mal_device* pDevice);

typedef enum
{
	mal_device_type_playback,
	mal_device_type_capture
} mal_device_type;

typedef enum
{
	mal_share_mode_shared = 0,
	mal_share_mode_exclusive,
} mal_share_mode;

typedef union
{
#ifdef MAL_SUPPORT_WASAPI
	wchar_t wasapi[64];             // WASAPI uses a wchar_t string for identification.
#endif
#ifdef MAL_SUPPORT_DSOUND
	mal_uint8 dsound[16];           // DirectSound uses a GUID for identification.
#endif
#ifdef MAL_SUPPORT_WINMM
	/*UINT_PTR*/ mal_uint32 winmm;  // When creating a device, WinMM expects a Win32 UINT_PTR for device identification. In practice it's actually just a UINT.
#endif
#ifdef MAL_SUPPORT_ALSA
	char alsa[256];                 // ALSA uses a name string for identification.
#endif
#ifdef MAL_SUPPORT_PULSEAUDIO
	char pulse[256];                // PulseAudio uses a name string for identification.
#endif
#ifdef MAL_SUPPORT_JACK
	int jack;                       // JACK always uses default devices.
#endif
#ifdef MAL_SUPPORT_COREAUDIO
	char coreaudio[256];            // Core Audio uses a string for identification.
#endif
#ifdef MAL_SUPPORT_SNDIO
	char sndio[256];                // "snd/0", etc.
#endif
#ifdef MAL_SUPPORT_AUDIO4
	char audio4[256];               // "/dev/audio", etc.
#endif
#ifdef MAL_SUPPORT_OSS
	char oss[64];                   // "dev/dsp0", etc. "dev/dsp" for the default device.
#endif
#ifdef MAL_SUPPORT_OPENSL
	mal_uint32 opensl;              // OpenSL|ES uses a 32-bit unsigned integer for identification.
#endif
#ifdef MAL_SUPPORT_OPENAL
	char openal[256];               // OpenAL seems to use human-readable device names as the ID.
#endif
#ifdef MAL_SUPPORT_SDL
	int sdl;                        // SDL devices are identified with an index.
#endif
#ifdef MAL_SUPPORT_NULL
	int nullbackend;                // The null backend uses an integer for device IDs.
#endif
} mal_device_id;

typedef struct
{
	// Basic info. This is the only information guaranteed to be filled in during device enumeration.
	mal_device_id id;
	char name[256];

	// Detailed info. As much of this is filled as possible with mal_context_get_device_info(). Note that you are allowed to initialize
	// a device with settings outside of this range, but it just means the data will be converted using mini_al's data conversion
	// pipeline before sending the data to/from the device. Most programs will need to not worry about these values, but it's provided
	// here mainly for informational purposes or in the rare case that someone might find it useful.
	//
	// These will be set to 0 when returned by mal_context_enumerate_devices() or mal_context_get_devices().
	mal_uint32 formatCount;
	mal_format formats[mal_format_count];
	mal_uint32 minChannels;
	mal_uint32 maxChannels;
	mal_uint32 minSampleRate;
	mal_uint32 maxSampleRate;
} mal_device_info;

typedef struct
{
	mal_int64 counter;
} mal_timer;

typedef struct
{
	mal_format format;
	mal_uint32 channels;
	mal_uint32 sampleRate;
	mal_channel channelMap[MAL_MAX_CHANNELS];
	mal_uint32 bufferSizeInFrames;
	mal_uint32 bufferSizeInMilliseconds;
	mal_uint32 periods;
	mal_share_mode shareMode;
	mal_performance_profile performanceProfile;
	mal_recv_proc onRecvCallback;
	mal_send_proc onSendCallback;
	mal_stop_proc onStopCallback;

	struct
	{
		mal_bool32 noMMap;  // Disables MMap mode.
	} alsa;

	struct
	{
		const char* pStreamName;
	} pulse;
} mal_device_config;

typedef struct
{
	mal_log_proc onLog;
	mal_thread_priority threadPriority;

	struct
	{
		mal_bool32 useVerboseDeviceEnumeration;
	} alsa;

	struct
	{
		const char* pApplicationName;
		const char* pServerName;
		mal_bool32 tryAutoSpawn; // Enables autospawning of the PulseAudio daemon if necessary.
	} pulse;

	struct
	{
		const char* pClientName;
		mal_bool32 tryStartServer;
	} jack;
} mal_context_config;

typedef mal_bool32(*mal_enum_devices_callback_proc)(mal_context* pContext, mal_device_type type, const mal_device_info* pInfo, void* pUserData);

struct mal_context
{
	mal_backend backend;                    // DirectSound, ALSA, etc.
	mal_context_config config;
	mal_mutex deviceEnumLock;               // Used to make mal_context_get_devices() thread safe.
	mal_mutex deviceInfoLock;               // Used to make mal_context_get_device_info() thread safe.
	mal_uint32 deviceInfoCapacity;          // Total capacity of pDeviceInfos.
	mal_uint32 playbackDeviceInfoCount;
	mal_uint32 captureDeviceInfoCount;
	mal_device_info* pDeviceInfos;          // Playback devices first, then capture.
	mal_bool32 isBackendAsynchronous : 1;   // Set when the context is initialized. Set to 1 for asynchronous backends such as Core Audio and JACK. Do not modify.

	mal_result(*onUninit)(mal_context* pContext);
	mal_bool32(*onDeviceIDEqual)(mal_context* pContext, const mal_device_id* pID0, const mal_device_id* pID1);
	mal_result(*onEnumDevices)(mal_context* pContext, mal_enum_devices_callback_proc callback, void* pUserData);    // Return false from the callback to stop enumeration.
	mal_result(*onGetDeviceInfo)(mal_context* pContext, mal_device_type type, const mal_device_id* pDeviceID, mal_share_mode shareMode, mal_device_info* pDeviceInfo);
	mal_result(*onDeviceInit)(mal_context* pContext, mal_device_type type, const mal_device_id* pDeviceID, const mal_device_config* pConfig, mal_device* pDevice);
	void(*onDeviceUninit)(mal_device* pDevice);
	mal_result(*onDeviceReinit)(mal_device* pDevice);
	mal_result(*onDeviceStart)(mal_device* pDevice);
	mal_result(*onDeviceStop)(mal_device* pDevice);
	mal_result(*onDeviceBreakMainLoop)(mal_device* pDevice);
	mal_result(*onDeviceMainLoop)(mal_device* pDevice);

	union
	{
#ifdef MAL_SUPPORT_WASAPI
		struct
		{
			int _unused;
		} wasapi;
#endif
#ifdef MAL_SUPPORT_DSOUND
		struct
		{
			/*HMODULE*/ mal_handle hDSoundDLL;
			mal_proc DirectSoundCreate;
			mal_proc DirectSoundEnumerateA;
			mal_proc DirectSoundCaptureCreate;
			mal_proc DirectSoundCaptureEnumerateA;
		} dsound;
#endif
#ifdef MAL_SUPPORT_WINMM
		struct
		{
			/*HMODULE*/ mal_handle hWinMM;
			mal_proc waveOutGetNumDevs;
			mal_proc waveOutGetDevCapsA;
			mal_proc waveOutOpen;
			mal_proc waveOutClose;
			mal_proc waveOutPrepareHeader;
			mal_proc waveOutUnprepareHeader;
			mal_proc waveOutWrite;
			mal_proc waveOutReset;
			mal_proc waveInGetNumDevs;
			mal_proc waveInGetDevCapsA;
			mal_proc waveInOpen;
			mal_proc waveInClose;
			mal_proc waveInPrepareHeader;
			mal_proc waveInUnprepareHeader;
			mal_proc waveInAddBuffer;
			mal_proc waveInStart;
			mal_proc waveInReset;
		} winmm;
#endif
#ifdef MAL_SUPPORT_ALSA
		struct
		{
			mal_handle asoundSO;
			mal_proc snd_pcm_open;
			mal_proc snd_pcm_close;
			mal_proc snd_pcm_hw_params_sizeof;
			mal_proc snd_pcm_hw_params_any;
			mal_proc snd_pcm_hw_params_set_format;
			mal_proc snd_pcm_hw_params_set_format_first;
			mal_proc snd_pcm_hw_params_get_format_mask;
			mal_proc snd_pcm_hw_params_set_channels_near;
			mal_proc snd_pcm_hw_params_set_rate_resample;
			mal_proc snd_pcm_hw_params_set_rate_near;
			mal_proc snd_pcm_hw_params_set_buffer_size_near;
			mal_proc snd_pcm_hw_params_set_periods_near;
			mal_proc snd_pcm_hw_params_set_access;
			mal_proc snd_pcm_hw_params_get_format;
			mal_proc snd_pcm_hw_params_get_channels;
			mal_proc snd_pcm_hw_params_get_channels_min;
			mal_proc snd_pcm_hw_params_get_channels_max;
			mal_proc snd_pcm_hw_params_get_rate;
			mal_proc snd_pcm_hw_params_get_rate_min;
			mal_proc snd_pcm_hw_params_get_rate_max;
			mal_proc snd_pcm_hw_params_get_buffer_size;
			mal_proc snd_pcm_hw_params_get_periods;
			mal_proc snd_pcm_hw_params_get_access;
			mal_proc snd_pcm_hw_params;
			mal_proc snd_pcm_sw_params_sizeof;
			mal_proc snd_pcm_sw_params_current;
			mal_proc snd_pcm_sw_params_set_avail_min;
			mal_proc snd_pcm_sw_params_set_start_threshold;
			mal_proc snd_pcm_sw_params;
			mal_proc snd_pcm_format_mask_sizeof;
			mal_proc snd_pcm_format_mask_test;
			mal_proc snd_pcm_get_chmap;
			mal_proc snd_pcm_prepare;
			mal_proc snd_pcm_start;
			mal_proc snd_pcm_drop;
			mal_proc snd_device_name_hint;
			mal_proc snd_device_name_get_hint;
			mal_proc snd_card_get_index;
			mal_proc snd_device_name_free_hint;
			mal_proc snd_pcm_mmap_begin;
			mal_proc snd_pcm_mmap_commit;
			mal_proc snd_pcm_recover;
			mal_proc snd_pcm_readi;
			mal_proc snd_pcm_writei;
			mal_proc snd_pcm_avail;
			mal_proc snd_pcm_avail_update;
			mal_proc snd_pcm_wait;
			mal_proc snd_pcm_info;
			mal_proc snd_pcm_info_sizeof;
			mal_proc snd_pcm_info_get_name;
			mal_proc snd_config_update_free_global;

			mal_mutex internalDeviceEnumLock;
		} alsa;
#endif
#ifdef MAL_SUPPORT_PULSEAUDIO
		struct
		{
			mal_handle pulseSO;
			mal_proc pa_mainloop_new;
			mal_proc pa_mainloop_free;
			mal_proc pa_mainloop_get_api;
			mal_proc pa_mainloop_iterate;
			mal_proc pa_mainloop_wakeup;
			mal_proc pa_context_new;
			mal_proc pa_context_unref;
			mal_proc pa_context_connect;
			mal_proc pa_context_disconnect;
			mal_proc pa_context_set_state_callback;
			mal_proc pa_context_get_state;
			mal_proc pa_context_get_sink_info_list;
			mal_proc pa_context_get_source_info_list;
			mal_proc pa_context_get_sink_info_by_name;
			mal_proc pa_context_get_source_info_by_name;
			mal_proc pa_operation_unref;
			mal_proc pa_operation_get_state;
			mal_proc pa_channel_map_init_extend;
			mal_proc pa_channel_map_valid;
			mal_proc pa_channel_map_compatible;
			mal_proc pa_stream_new;
			mal_proc pa_stream_unref;
			mal_proc pa_stream_connect_playback;
			mal_proc pa_stream_connect_record;
			mal_proc pa_stream_disconnect;
			mal_proc pa_stream_get_state;
			mal_proc pa_stream_get_sample_spec;
			mal_proc pa_stream_get_channel_map;
			mal_proc pa_stream_get_buffer_attr;
			mal_proc pa_stream_set_buffer_attr;
			mal_proc pa_stream_get_device_name;
			mal_proc pa_stream_set_write_callback;
			mal_proc pa_stream_set_read_callback;
			mal_proc pa_stream_flush;
			mal_proc pa_stream_drain;
			mal_proc pa_stream_cork;
			mal_proc pa_stream_trigger;
			mal_proc pa_stream_begin_write;
			mal_proc pa_stream_write;
			mal_proc pa_stream_peek;
			mal_proc pa_stream_drop;
		} pulse;
#endif
#ifdef MAL_SUPPORT_JACK
		struct
		{
			mal_handle jackSO;
			mal_proc jack_client_open;
			mal_proc jack_client_close;
			mal_proc jack_client_name_size;
			mal_proc jack_set_process_callback;
			mal_proc jack_set_buffer_size_callback;
			mal_proc jack_on_shutdown;
			mal_proc jack_get_sample_rate;
			mal_proc jack_get_buffer_size;
			mal_proc jack_get_ports;
			mal_proc jack_activate;
			mal_proc jack_deactivate;
			mal_proc jack_connect;
			mal_proc jack_port_register;
			mal_proc jack_port_name;
			mal_proc jack_port_get_buffer;
			mal_proc jack_free;
		} jack;
#endif
#ifdef MAL_SUPPORT_COREAUDIO
		struct
		{
			mal_handle hCoreFoundation;
			mal_proc CFStringGetCString;

			mal_handle hCoreAudio;
			mal_proc AudioObjectGetPropertyData;
			mal_proc AudioObjectGetPropertyDataSize;
			mal_proc AudioObjectSetPropertyData;
			mal_proc AudioObjectAddPropertyListener;

			mal_handle hAudioUnit;  // Could possibly be set to AudioToolbox on later versions of macOS.
			mal_proc AudioComponentFindNext;
			mal_proc AudioComponentInstanceDispose;
			mal_proc AudioComponentInstanceNew;
			mal_proc AudioOutputUnitStart;
			mal_proc AudioOutputUnitStop;
			mal_proc AudioUnitAddPropertyListener;
			mal_proc AudioUnitGetPropertyInfo;
			mal_proc AudioUnitGetProperty;
			mal_proc AudioUnitSetProperty;
			mal_proc AudioUnitInitialize;
			mal_proc AudioUnitRender;
		} coreaudio;
#endif
#ifdef MAL_SUPPORT_SNDIO
		struct
		{
			mal_handle sndioSO;
			mal_proc sio_open;
			mal_proc sio_close;
			mal_proc sio_setpar;
			mal_proc sio_getpar;
			mal_proc sio_getcap;
			mal_proc sio_start;
			mal_proc sio_stop;
			mal_proc sio_read;
			mal_proc sio_write;
			mal_proc sio_onmove;
			mal_proc sio_nfds;
			mal_proc sio_pollfd;
			mal_proc sio_revents;
			mal_proc sio_eof;
			mal_proc sio_setvol;
			mal_proc sio_onvol;
			mal_proc sio_initpar;
		} sndio;
#endif
#ifdef MAL_SUPPORT_AUDIO4
		struct
		{
			int _unused;
		} audio4;
#endif
#ifdef MAL_SUPPORT_OSS
		struct
		{
			int versionMajor;
			int versionMinor;
		} oss;
#endif
#ifdef MAL_SUPPORT_OPENSL
		struct
		{
			int _unused;
		} opensl;
#endif
#ifdef MAL_SUPPORT_OPENAL
		struct
		{
			/*HMODULE*/ mal_handle hOpenAL;     // OpenAL32.dll, etc.
			mal_proc alcCreateContext;
			mal_proc alcMakeContextCurrent;
			mal_proc alcProcessContext;
			mal_proc alcSuspendContext;
			mal_proc alcDestroyContext;
			mal_proc alcGetCurrentContext;
			mal_proc alcGetContextsDevice;
			mal_proc alcOpenDevice;
			mal_proc alcCloseDevice;
			mal_proc alcGetError;
			mal_proc alcIsExtensionPresent;
			mal_proc alcGetProcAddress;
			mal_proc alcGetEnumValue;
			mal_proc alcGetString;
			mal_proc alcGetIntegerv;
			mal_proc alcCaptureOpenDevice;
			mal_proc alcCaptureCloseDevice;
			mal_proc alcCaptureStart;
			mal_proc alcCaptureStop;
			mal_proc alcCaptureSamples;
			mal_proc alEnable;
			mal_proc alDisable;
			mal_proc alIsEnabled;
			mal_proc alGetString;
			mal_proc alGetBooleanv;
			mal_proc alGetIntegerv;
			mal_proc alGetFloatv;
			mal_proc alGetDoublev;
			mal_proc alGetBoolean;
			mal_proc alGetInteger;
			mal_proc alGetFloat;
			mal_proc alGetDouble;
			mal_proc alGetError;
			mal_proc alIsExtensionPresent;
			mal_proc alGetProcAddress;
			mal_proc alGetEnumValue;
			mal_proc alGenSources;
			mal_proc alDeleteSources;
			mal_proc alIsSource;
			mal_proc alSourcef;
			mal_proc alSource3f;
			mal_proc alSourcefv;
			mal_proc alSourcei;
			mal_proc alSource3i;
			mal_proc alSourceiv;
			mal_proc alGetSourcef;
			mal_proc alGetSource3f;
			mal_proc alGetSourcefv;
			mal_proc alGetSourcei;
			mal_proc alGetSource3i;
			mal_proc alGetSourceiv;
			mal_proc alSourcePlayv;
			mal_proc alSourceStopv;
			mal_proc alSourceRewindv;
			mal_proc alSourcePausev;
			mal_proc alSourcePlay;
			mal_proc alSourceStop;
			mal_proc alSourceRewind;
			mal_proc alSourcePause;
			mal_proc alSourceQueueBuffers;
			mal_proc alSourceUnqueueBuffers;
			mal_proc alGenBuffers;
			mal_proc alDeleteBuffers;
			mal_proc alIsBuffer;
			mal_proc alBufferData;
			mal_proc alBufferf;
			mal_proc alBuffer3f;
			mal_proc alBufferfv;
			mal_proc alBufferi;
			mal_proc alBuffer3i;
			mal_proc alBufferiv;
			mal_proc alGetBufferf;
			mal_proc alGetBuffer3f;
			mal_proc alGetBufferfv;
			mal_proc alGetBufferi;
			mal_proc alGetBuffer3i;
			mal_proc alGetBufferiv;

			mal_bool32 isEnumerationSupported : 1;
			mal_bool32 isFloat32Supported : 1;
			mal_bool32 isMCFormatsSupported : 1;
		} openal;
#endif
#ifdef MAL_SUPPORT_SDL
		struct
		{
			mal_handle hSDL;    // SDL
			mal_proc SDL_InitSubSystem;
			mal_proc SDL_QuitSubSystem;
			mal_proc SDL_GetNumAudioDevices;
			mal_proc SDL_GetAudioDeviceName;
			mal_proc SDL_CloseAudioDevice;
			mal_proc SDL_OpenAudioDevice;
			mal_proc SDL_PauseAudioDevice;
		} sdl;
#endif
#ifdef MAL_SUPPORT_NULL
		struct
		{
			int _unused;
		} null_backend;
#endif
	};

	union
	{
#ifdef MAL_WIN32
		struct
		{
			/*HMODULE*/ mal_handle hOle32DLL;
			mal_proc CoInitializeEx;
			mal_proc CoUninitialize;
			mal_proc CoCreateInstance;
			mal_proc CoTaskMemFree;
			mal_proc PropVariantClear;
			mal_proc StringFromGUID2;

			/*HMODULE*/ mal_handle hUser32DLL;
			mal_proc GetForegroundWindow;
			mal_proc GetDesktopWindow;

			/*HMODULE*/ mal_handle hAdvapi32DLL;
			mal_proc RegOpenKeyExA;
			mal_proc RegCloseKey;
			mal_proc RegQueryValueExA;
		} win32;
#endif
#ifdef MAL_POSIX
		struct
		{
			mal_handle pthreadSO;
			mal_proc pthread_create;
			mal_proc pthread_join;
			mal_proc pthread_mutex_init;
			mal_proc pthread_mutex_destroy;
			mal_proc pthread_mutex_lock;
			mal_proc pthread_mutex_unlock;
			mal_proc pthread_cond_init;
			mal_proc pthread_cond_destroy;
			mal_proc pthread_cond_wait;
			mal_proc pthread_cond_signal;
			mal_proc pthread_attr_init;
			mal_proc pthread_attr_destroy;
			mal_proc pthread_attr_setschedpolicy;
			mal_proc pthread_attr_getschedparam;
			mal_proc pthread_attr_setschedparam;
		} posix;
#endif
		int _unused;
	};
};


MAL_ALIGNED_STRUCT(MAL_SIMD_ALIGNMENT) mal_device
{
	mal_context* pContext;
	mal_device_type type;
	mal_format format;
	mal_uint32 channels;
	mal_uint32 sampleRate;
	mal_channel channelMap[MAL_MAX_CHANNELS];
	mal_uint32 bufferSizeInFrames;
	mal_uint32 bufferSizeInMilliseconds;
	mal_uint32 periods;
	mal_uint32 state;
	mal_recv_proc onRecv;
	mal_send_proc onSend;
	mal_stop_proc onStop;
	void* pUserData;                // Application defined data.
	char name[256];
	mal_device_config initConfig;   // The configuration passed in to mal_device_init(). Mainly used for reinitializing the backend device.
	mal_mutex lock;
	mal_event wakeupEvent;
	mal_event startEvent;
	mal_event stopEvent;
	mal_thread thread;
	mal_result workResult;          // This is set by the worker thread after it's finished doing a job.
	mal_bool32 usingDefaultFormat : 1;
	mal_bool32 usingDefaultChannels : 1;
	mal_bool32 usingDefaultSampleRate : 1;
	mal_bool32 usingDefaultChannelMap : 1;
	mal_bool32 usingDefaultBufferSize : 1;
	mal_bool32 usingDefaultPeriods : 1;
	mal_bool32 exclusiveMode : 1;
	mal_bool32 isOwnerOfContext : 1;  // When set to true, uninitializing the device will also uninitialize the context. Set to true when NULL is passed into mal_device_init().
	mal_bool32 isDefaultDevice : 1;  // Used to determine if the backend should try reinitializing if the default device is unplugged.
	mal_format internalFormat;
	mal_uint32 internalChannels;
	mal_uint32 internalSampleRate;
	mal_channel internalChannelMap[MAL_MAX_CHANNELS];
	mal_dsp dsp;                    // Samples run through this to convert samples to a format suitable for use by the backend.
	mal_uint32 _dspFrameCount;      // Internal use only. Used when running the device -> DSP -> client pipeline. See mal_device__on_read_from_device().
	const mal_uint8* _dspFrames;    // ^^^ AS ABOVE ^^^

	union
	{
#ifdef MAL_SUPPORT_WASAPI
		struct
		{
			/*IAudioClient**/ mal_ptr pAudioClient;
			/*IAudioRenderClient**/ mal_ptr pRenderClient;
			/*IAudioCaptureClient**/ mal_ptr pCaptureClient;
			/*IMMDeviceEnumerator**/ mal_ptr pDeviceEnumerator; /* <-- Used for IMMNotificationClient notifications. Required for detecting default device changes. */
			mal_IMMNotificationClient notificationClient;
			/*HANDLE*/ mal_handle hEvent;
			/*HANDLE*/ mal_handle hBreakEvent;  /* <-- Used to break from WaitForMultipleObjects() in the main loop. */
			mal_bool32 breakFromMainLoop;
			mal_bool32 hasDefaultDeviceChanged; /* <-- Make sure this is always a whole 32-bits because we use atomic assignments. */
		} wasapi;
#endif
#ifdef MAL_SUPPORT_DSOUND
		struct
		{
			/*LPDIRECTSOUND*/ mal_ptr pPlayback;
			/*LPDIRECTSOUNDBUFFER*/ mal_ptr pPlaybackPrimaryBuffer;
			/*LPDIRECTSOUNDBUFFER*/ mal_ptr pPlaybackBuffer;
			/*LPDIRECTSOUNDCAPTURE*/ mal_ptr pCapture;
			/*LPDIRECTSOUNDCAPTUREBUFFER*/ mal_ptr pCaptureBuffer;
			/*LPDIRECTSOUNDNOTIFY*/ mal_ptr pNotify;
			/*HANDLE*/ mal_handle pNotifyEvents[MAL_MAX_PERIODS_DSOUND];  // One event handle for each period.
			/*HANDLE*/ mal_handle hStopEvent;
			mal_uint32 lastProcessedFrame;      // This is circular.
			mal_bool32 breakFromMainLoop;
		} dsound;
#endif
#ifdef MAL_SUPPORT_WINMM
		struct
		{
			/*HWAVEOUT, HWAVEIN*/ mal_handle hDevice;
			/*HANDLE*/ mal_handle hEvent;
			mal_uint32 fragmentSizeInFrames;
			mal_uint32 fragmentSizeInBytes;
			mal_uint32 iNextHeader;             // [0,periods). Used as an index into pWAVEHDR.
			/*WAVEHDR**/ mal_uint8* pWAVEHDR;   // One instantiation for each period.
			mal_uint8* pIntermediaryBuffer;
			mal_uint8* _pHeapData;              // Used internally and is used for the heap allocated data for the intermediary buffer and the WAVEHDR structures.
			mal_bool32 breakFromMainLoop;
		} winmm;
#endif
#ifdef MAL_SUPPORT_ALSA
		struct
		{
			/*snd_pcm_t**/ mal_ptr pPCM;
			mal_bool32 isUsingMMap : 1;
			mal_bool32 breakFromMainLoop : 1;
			void* pIntermediaryBuffer;
		} alsa;
#endif
#ifdef MAL_SUPPORT_PULSEAUDIO
		struct
		{
			/*pa_mainloop**/ mal_ptr pMainLoop;
			/*pa_mainloop_api**/ mal_ptr pAPI;
			/*pa_context**/ mal_ptr pPulseContext;
			/*pa_stream**/ mal_ptr pStream;
			/*pa_context_state*/ mal_uint32 pulseContextState;
			mal_uint32 fragmentSizeInBytes;
			mal_bool32 breakFromMainLoop : 1;
		} pulse;
#endif
#ifdef MAL_SUPPORT_JACK
		struct
		{
			/*jack_client_t**/ mal_ptr pClient;
			/*jack_port_t**/ mal_ptr pPorts[MAL_MAX_CHANNELS];
			float* pIntermediaryBuffer; // Typed as a float because JACK is always floating point.
		} jack;
#endif
#ifdef MAL_SUPPORT_COREAUDIO
		struct
		{
			mal_uint32 deviceObjectID;
			/*AudioComponent*/ mal_ptr component;   // <-- Can this be per-context?
			/*AudioUnit*/ mal_ptr audioUnit;
			/*AudioBufferList**/ mal_ptr pAudioBufferList;  // Only used for input devices.
			mal_event stopEvent;
			mal_bool32 isSwitchingDevice;   /* <-- Set to true when the default device has changed and mini_al is in the process of switching. */
		} coreaudio;
#endif
#ifdef MAL_SUPPORT_SNDIO
		struct
		{
			mal_ptr handle;
			mal_uint32 fragmentSizeInFrames;
			mal_bool32 breakFromMainLoop;
			void* pIntermediaryBuffer;
		} sndio;
#endif
#ifdef MAL_SUPPORT_AUDIO4
		struct
		{
			int fd;
			mal_uint32 fragmentSizeInFrames;
			mal_bool32 breakFromMainLoop;
			void* pIntermediaryBuffer;
		} audio4;
#endif
#ifdef MAL_SUPPORT_OSS
		struct
		{
			int fd;
			mal_uint32 fragmentSizeInFrames;
			mal_bool32 breakFromMainLoop;
			void* pIntermediaryBuffer;
		} oss;
#endif
#ifdef MAL_SUPPORT_OPENSL
		struct
		{
			/*SLObjectItf*/ mal_ptr pOutputMixObj;
			/*SLOutputMixItf*/ mal_ptr pOutputMix;
			/*SLObjectItf*/ mal_ptr pAudioPlayerObj;
			/*SLPlayItf*/ mal_ptr pAudioPlayer;
			/*SLObjectItf*/ mal_ptr pAudioRecorderObj;
			/*SLRecordItf*/ mal_ptr pAudioRecorder;
			/*SLAndroidSimpleBufferQueueItf*/ mal_ptr pBufferQueue;
			mal_uint32 periodSizeInFrames;
			mal_uint32 currentBufferIndex;
			mal_uint8* pBuffer;                 // This is malloc()'d and is used for storing audio data. Typed as mal_uint8 for easy offsetting.
		} opensl;
#endif
#ifdef MAL_SUPPORT_OPENAL
		struct
		{
			/*ALCcontext**/ mal_ptr pContextALC;
			/*ALCdevice**/ mal_ptr pDeviceALC;
			/*ALuint*/ mal_uint32 sourceAL;
			/*ALuint*/ mal_uint32 buffersAL[MAL_MAX_PERIODS_OPENAL];
			/*ALenum*/ mal_uint32 formatAL;
			mal_uint32 subBufferSizeInFrames;   // This is the size of each of the OpenAL buffers (buffersAL).
			mal_uint8* pIntermediaryBuffer;     // This is malloc()'d and is used as the destination for reading from the client. Typed as mal_uint8 for easy offsetting.
			mal_uint32 iNextBuffer;             // The next buffer to unenqueue and then re-enqueue as new data is read.
			mal_bool32 breakFromMainLoop;
		} openal;
#endif
#ifdef MAL_SUPPORT_SDL
		struct
		{
			mal_uint32 deviceID;
		} sdl;
#endif
#ifdef MAL_SUPPORT_NULL
		struct
		{
			mal_timer timer;
			mal_uint32 lastProcessedFrame;      // This is circular.
			mal_bool32 breakFromMainLoop;
			mal_uint8* pBuffer;                 // This is malloc()'d and is used as the destination for reading from the client. Typed as mal_uint8 for easy offsetting.
		} null_device;
#endif
	};
};
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

// Initializes a context.
//
// The context is used for selecting and initializing the relevant backends.
//
// Note that the location of the context cannot change throughout it's lifetime. Consider allocating
// the mal_context object with malloc() if this is an issue. The reason for this is that a pointer
// to the context is stored in the mal_device structure.
//
// <backends> is used to allow the application to prioritize backends depending on it's specific
// requirements. This can be null in which case it uses the default priority, which is as follows:
//   - WASAPI
//   - DirectSound
//   - WinMM
//   - Core Audio (Apple)
//   - sndio
//   - audio(4)
//   - OSS
//   - PulseAudio
//   - ALSA
//   - JACK
//   - OpenSL|ES
//   - OpenAL
//   - SDL
//   - Null
//
// <pConfig> is used to configure the context. Use the onLog config to set a callback for whenever a
// log message is posted. The priority of the worker thread can be set with the threadPriority config.
//
// It is recommended that only a single context is active at any given time because it's a bulky data
// structure which performs run-time linking for the relevant backends every time it's initialized.
//
// Return Value:
//   MAL_SUCCESS if successful; any other error code otherwise.
//
// Thread Safety: UNSAFE
mal_result mal_context_init(const mal_backend backends[], mal_uint32 backendCount, const mal_context_config* pConfig, mal_context* pContext);

// Uninitializes a context.
//
// Results are undefined if you call this while any device created by this context is still active.
//
// Return Value:
//   MAL_SUCCESS if successful; any other error code otherwise.
//
// Thread Safety: UNSAFE
mal_result mal_context_uninit(mal_context* pContext);

// Enumerates over every device (both playback and capture).
//
// This is a lower-level enumeration function to the easier to use mal_context_get_devices(). Use
// mal_context_enumerate_devices() if you would rather not incur an internal heap allocation, or
// it simply suits your code better.
//
// Do _not_ assume the first enumerated device of a given type is the default device.
//
// Some backends and platforms may only support default playback and capture devices.
//
// Note that this only retrieves the ID and name/description of the device. The reason for only
// retrieving basic information is that it would otherwise require opening the backend device in
// order to probe it for more detailed information which can be inefficient. Consider using
// mal_context_get_device_info() for this, but don't call it from within the enumeration callback.
//
// In general, you should not do anything complicated from within the callback. In particular, do
// not try initializing a device from within the callback.
//
// Consider using mal_context_get_devices() for a simpler and safer API, albeit at the expense of
// an internal heap allocation.
//
// Returning false from the callback will stop enumeration. Returning true will continue enumeration.
//
// Return Value:
//   MAL_SUCCESS if successful; any other error code otherwise.
//
// Thread Safety: SAFE
//   This is guarded using a simple mutex lock.
mal_result mal_context_enumerate_devices(mal_context* pContext, mal_enum_devices_callback_proc callback, void* pUserData);

// Retrieves basic information about every active playback and/or capture device.
//
// You can pass in NULL for the playback or capture lists in which case they'll be ignored.
//
// It is _not_ safe to assume the first device in the list is the default device.
//
// The returned pointers will become invalid upon the next call this this function, or when the
// context is uninitialized. Do not free the returned pointers.
//
// This function follows the same enumeration rules as mal_context_enumerate_devices(). See
// documentation for mal_context_enumerate_devices() for more information.
//
// Return Value:
//   MAL_SUCCESS if successful; any other error code otherwise.
//
// Thread Safety: SAFE
//   Since each call to this function invalidates the pointers from the previous call, you
//   should not be calling this simultaneously across multiple threads. Instead, you need to
//   make a copy of the returned data with your own higher level synchronization.
mal_result mal_context_get_devices(mal_context* pContext, mal_device_info** ppPlaybackDeviceInfos, mal_uint32* pPlaybackDeviceCount, mal_device_info** ppCaptureDeviceInfos, mal_uint32* pCaptureDeviceCount);

// Retrieves information about a device with the given ID.
//
// Do _not_ call this from within the mal_context_enumerate_devices() callback.
//
// It's possible for a device to have different information and capabilities depending on wether or
// not it's opened in shared or exclusive mode. For example, in shared mode, WASAPI always uses
// floating point samples for mixing, but in exclusive mode it can be anything. Therefore, this
// function allows you to specify which share mode you want information for. Note that not all
// backends and devices support shared or exclusive mode, in which case this function will fail
// if the requested share mode is unsupported.
//
// This leaves pDeviceInfo unmodified in the result of an error.
//
// Return Value:
//   MAL_SUCCESS if successful; any other error code otherwise.
//
// Thread Safety: SAFE
//   This is guarded using a simple mutex lock.
mal_result mal_context_get_device_info(mal_context* pContext, mal_device_type type, const mal_device_id* pDeviceID, mal_share_mode shareMode, mal_device_info* pDeviceInfo);

// Initializes a device.
//
// The context can be null in which case it uses the default. This is equivalent to passing in a
// context that was initialized like so:
//
//     mal_context_init(NULL, 0, NULL, &context);
//
// Do not pass in null for the context if you are needing to open multiple devices. You can,
// however, use null when initializing the first device, and then use device.pContext for the
// initialization of other devices.
//
// The device ID (pDeviceID) can be null, in which case the default device is used. Otherwise, you
// can retrieve the ID by calling mal_context_get_devices() and using the ID from the returned data.
// Set pDeviceID to NULL to use the default device. Do _not_ rely on the first device ID returned
// by mal_context_enumerate_devices() or mal_context_get_devices() to be the default device.
//
// The device's configuration is controlled with pConfig. This allows you to configure the sample
// format, channel count, sample rate, etc. Before calling mal_device_init(), you will most likely
// want to initialize a mal_device_config object using mal_device_config_init(),
// mal_device_config_init_playback(), etc. You can also pass in NULL for the device config in
// which case it will use defaults, but will require you to call mal_device_set_recv_callback() or
// mal_device_set_send_callback() before starting the device.
//
// Passing in 0 to any property in pConfig will force the use of a default value. In the case of
// sample format, channel count, sample rate and channel map it will default to the values used by
// the backend's internal device. For the size of the buffer you can set bufferSizeInFrames or
// bufferSizeInMilliseconds (if both are set it will prioritize bufferSizeInFrames). If both are
// set to zero, it will default to MAL_BASE_BUFFER_SIZE_IN_MILLISECONDS_LOW_LATENCY or
// MAL_BASE_BUFFER_SIZE_IN_MILLISECONDS_CONSERVATIVE, depending on whether or not performanceProfile
// is set to mal_performance_profile_low_latency or mal_performance_profile_conservative.
//
// When sending or receiving data to/from a device, mini_al will internally perform a format
// conversion to convert between the format specified by pConfig and the format used internally by
// the backend. If you pass in NULL for pConfig or 0 for the sample format, channel count,
// sample rate _and_ channel map, data transmission will run on an optimized pass-through fast path.
//
// The <periods> property controls how frequently the background thread is woken to check for more
// data. It's tied to the buffer size, so as an example, if your buffer size is equivalent to 10
// milliseconds and you have 2 periods, the CPU will wake up approximately every 5 milliseconds.
//
// When compiling for UWP you must ensure you call this function on the main UI thread because the
// operating system may need to present the user with a message asking for permissions. Please refer
// to the official documentation for ActivateAudioInterfaceAsync() for more information.
//
// Return Value:
//   MAL_SUCCESS if successful; any other error code otherwise.
//
// Thread Safety: UNSAFE
//   It is not safe to call this function simultaneously for different devices because some backends
//   depend on and mutate global state (such as OpenSL|ES). The same applies to calling this at the
//   same time as mal_device_uninit().
mal_result mal_device_init(mal_context* pContext, mal_device_type type, mal_device_id* pDeviceID, const mal_device_config* pConfig, void* pUserData, mal_device* pDevice);

// Initializes a device without a context, with extra parameters for controlling the configuration
// of the internal self-managed context.
//
// See mal_device_init() and mal_context_init().
mal_result mal_device_init_ex(const mal_backend backends[], mal_uint32 backendCount, const mal_context_config* pContextConfig, mal_device_type type, mal_device_id* pDeviceID, const mal_device_config* pConfig, void* pUserData, mal_device* pDevice);

// Uninitializes a device.
//
// This will explicitly stop the device. You do not need to call mal_device_stop() beforehand, but it's
// harmless if you do.
//
// Return Value:
//   MAL_SUCCESS if successful; any other error code otherwise.
//
// Thread Safety: UNSAFE
//   As soon as this API is called the device should be considered undefined. All bets are off if you
//   try using the device at the same time as uninitializing it.
void mal_device_uninit(mal_device* pDevice);

// Sets the callback to use when the application has received data from the device.
//
// Thread Safety: SAFE
//   This API is implemented as a simple atomic assignment.
//
// DEPRECATED. Set this when the device is initialized with mal_device_init*().
void mal_device_set_recv_callback(mal_device* pDevice, mal_recv_proc proc);

// Sets the callback to use when the application needs to send data to the device for playback.
//
// Note that the implementation of this callback must copy over as many samples as is available. The
// return value specifies how many samples were written to the output buffer. The backend will fill
// any leftover samples with silence.
//
// Thread Safety: SAFE
//   This API is implemented as a simple atomic assignment.
//
// DEPRECATED. Set this when the device is initialized with mal_device_init*().
void mal_device_set_send_callback(mal_device* pDevice, mal_send_proc proc);

// Sets the callback to use when the device has stopped, either explicitly or as a result of an error.
//
// Thread Safety: SAFE
//   This API is implemented as a simple atomic assignment.
void mal_device_set_stop_callback(mal_device* pDevice, mal_stop_proc proc);

// Activates the device. For playback devices this begins playback. For capture devices it begins
// recording.
//
// For a playback device, this will retrieve an initial chunk of audio data from the client before
// returning. The reason for this is to ensure there is valid audio data in the buffer, which needs
// to be done _before_ the device begins playback.
//
// This API waits until the backend device has been started for real by the worker thread. It also
// waits on a mutex for thread-safety.
//
// Return Value:
//   - MAL_SUCCESS if successful; any other error code otherwise.
//   - MAL_INVALID_ARGS
//       One or more of the input arguments is invalid.
//   - MAL_DEVICE_NOT_INITIALIZED
//       The device is not currently or was never initialized.
//   - MAL_DEVICE_BUSY
//       The device is in the process of stopping. This will only happen if mal_device_start() and
//       mal_device_stop() is called simultaneous on separate threads. This will never be returned in
//       single-threaded applications.
//   - MAL_DEVICE_ALREADY_STARTING
//       The device is already in the process of starting. This will never be returned in single-threaded
//       applications.
//   - MAL_DEVICE_ALREADY_STARTED
//       The device is already started.
//   - MAL_FAILED_TO_READ_DATA_FROM_CLIENT
//       Failed to read the initial chunk of audio data from the client. This initial chunk of data is
//       required so that the device has valid audio data as soon as it starts playing. This will never
//       be returned for capture devices.
//   - MAL_FAILED_TO_START_BACKEND_DEVICE
//       There was a backend-specific error starting the device.
//
// Thread Safety: SAFE
mal_result mal_device_start(mal_device* pDevice);

// Puts the device to sleep, but does not uninitialize it. Use mal_device_start() to start it up again.
//
// This API needs to wait on the worker thread to stop the backend device properly before returning. It
// also waits on a mutex for thread-safety. In addition, some backends need to wait for the device to
// finish playback/recording of the current fragment which can take some time (usually proportionate to
// the buffer size that was specified at initialization time).
//
// Return Value:
//   - MAL_SUCCESS if successful; any other error code otherwise.
//   - MAL_INVALID_ARGS
//       One or more of the input arguments is invalid.
//   - MAL_DEVICE_NOT_INITIALIZED
//       The device is not currently or was never initialized.
//   - MAL_DEVICE_BUSY
//       The device is in the process of starting. This will only happen if mal_device_start() and
//       mal_device_stop() is called simultaneous on separate threads. This will never be returned in
//       single-threaded applications.
//   - MAL_DEVICE_ALREADY_STOPPING
//       The device is already in the process of stopping. This will never be returned in single-threaded
//       applications.
//   - MAL_DEVICE_ALREADY_STOPPED
//       The device is already stopped.
//   - MAL_FAILED_TO_STOP_BACKEND_DEVICE
//       There was a backend-specific error stopping the device.
//
// Thread Safety: SAFE
mal_result mal_device_stop(mal_device* pDevice);

// Determines whether or not the device is started.
//
// This is implemented as a simple accessor.
//
// Return Value:
//   True if the device is started, false otherwise.
//
// Thread Safety: SAFE
//   If another thread calls mal_device_start() or mal_device_stop() at this same time as this function
//   is called, there's a very small chance the return value will be out of sync.
mal_bool32 mal_device_is_started(mal_device* pDevice);

// Retrieves the size of the buffer in bytes for the given device.
//
// This API is efficient and is implemented with just a few 32-bit integer multiplications.
//
// Thread Safety: SAFE
//   This is calculated from constant values which are set at initialization time and never change.
mal_uint32 mal_device_get_buffer_size_in_bytes(mal_device* pDevice);


// Helper function for initializing a mal_context_config object.
mal_context_config mal_context_config_init(mal_log_proc onLog);

// Initializes a default device config.
//
// A default configuration will configure the device such that the format, channel count, sample rate and channel map are
// the same as the backend's internal configuration. This means the application loses explicit control of these properties,
// but in return gets an optimized fast path for data transmission since mini_al will be releived of all format conversion
// duties. You will not typically want to use default configurations unless you have some specific low-latency requirements.
//
// mal_device_config_init(), mal_device_config_init_playback(), etc. will allow you to explicitly set the sample format,
// channel count, etc.
mal_device_config mal_device_config_init_default(void);
mal_device_config mal_device_config_init_default_capture(mal_recv_proc onRecvCallback);
mal_device_config mal_device_config_init_default_playback(mal_send_proc onSendCallback);

// Helper function for initializing a mal_device_config object.
//
// This is just a helper API, and as such the returned object can be safely modified as needed.
//
// The default channel mapping is based on the channel count, as per the table below. Note that these
// can be freely changed after this function returns if you are needing something in particular.
//
// |---------------|------------------------------|
// | Channel Count | Mapping                      |
// |---------------|------------------------------|
// | 1 (Mono)      | 0: MAL_CHANNEL_MONO          |
// |---------------|------------------------------|
// | 2 (Stereo)    | 0: MAL_CHANNEL_FRONT_LEFT    |
// |               | 1: MAL_CHANNEL_FRONT_RIGHT   |
// |---------------|------------------------------|
// | 3             | 0: MAL_CHANNEL_FRONT_LEFT    |
// |               | 1: MAL_CHANNEL_FRONT_RIGHT   |
// |               | 2: MAL_CHANNEL_FRONT_CENTER  |
// |---------------|------------------------------|
// | 4 (Surround)  | 0: MAL_CHANNEL_FRONT_LEFT    |
// |               | 1: MAL_CHANNEL_FRONT_RIGHT   |
// |               | 2: MAL_CHANNEL_FRONT_CENTER  |
// |               | 3: MAL_CHANNEL_BACK_CENTER   |
// |---------------|------------------------------|
// | 5             | 0: MAL_CHANNEL_FRONT_LEFT    |
// |               | 1: MAL_CHANNEL_FRONT_RIGHT   |
// |               | 2: MAL_CHANNEL_FRONT_CENTER  |
// |               | 3: MAL_CHANNEL_BACK_LEFT     |
// |               | 4: MAL_CHANNEL_BACK_RIGHT    |
// |---------------|------------------------------|
// | 6 (5.1)       | 0: MAL_CHANNEL_FRONT_LEFT    |
// |               | 1: MAL_CHANNEL_FRONT_RIGHT   |
// |               | 2: MAL_CHANNEL_FRONT_CENTER  |
// |               | 3: MAL_CHANNEL_LFE           |
// |               | 4: MAL_CHANNEL_SIDE_LEFT     |
// |               | 5: MAL_CHANNEL_SIDE_RIGHT    |
// |---------------|------------------------------|
// | 7             | 0: MAL_CHANNEL_FRONT_LEFT    |
// |               | 1: MAL_CHANNEL_FRONT_RIGHT   |
// |               | 2: MAL_CHANNEL_FRONT_CENTER  |
// |               | 3: MAL_CHANNEL_LFE           |
// |               | 4: MAL_CHANNEL_BACK_CENTER   |
// |               | 4: MAL_CHANNEL_SIDE_LEFT     |
// |               | 5: MAL_CHANNEL_SIDE_RIGHT    |
// |---------------|------------------------------|
// | 8 (7.1)       | 0: MAL_CHANNEL_FRONT_LEFT    |
// |               | 1: MAL_CHANNEL_FRONT_RIGHT   |
// |               | 2: MAL_CHANNEL_FRONT_CENTER  |
// |               | 3: MAL_CHANNEL_LFE           |
// |               | 4: MAL_CHANNEL_BACK_LEFT     |
// |               | 5: MAL_CHANNEL_BACK_RIGHT    |
// |               | 6: MAL_CHANNEL_SIDE_LEFT     |
// |               | 7: MAL_CHANNEL_SIDE_RIGHT    |
// |---------------|------------------------------|
// | Other         | All channels set to 0. This  |
// |               | is equivalent to the same    |
// |               | mapping as the device.       |
// |---------------|------------------------------|
//
// Thread Safety: SAFE
//
// Efficiency: HIGH
//   This just returns a stack allocated object and consists of just a few assignments.
mal_device_config mal_device_config_init_ex(mal_format format, mal_uint32 channels, mal_uint32 sampleRate, mal_channel channelMap[MAL_MAX_CHANNELS], mal_recv_proc onRecvCallback, mal_send_proc onSendCallback);

// A simplified version of mal_device_config_init_ex().
static MAL_INLINE mal_device_config mal_device_config_init(mal_format format, mal_uint32 channels, mal_uint32 sampleRate, mal_recv_proc onRecvCallback, mal_send_proc onSendCallback) { return mal_device_config_init_ex(format, channels, sampleRate, NULL, onRecvCallback, onSendCallback); }

// A simplified version of mal_device_config_init() for capture devices.
static MAL_INLINE mal_device_config mal_device_config_init_capture_ex(mal_format format, mal_uint32 channels, mal_uint32 sampleRate, mal_channel channelMap[MAL_MAX_CHANNELS], mal_recv_proc onRecvCallback) { return mal_device_config_init_ex(format, channels, sampleRate, channelMap, onRecvCallback, NULL); }
static MAL_INLINE mal_device_config mal_device_config_init_capture(mal_format format, mal_uint32 channels, mal_uint32 sampleRate, mal_recv_proc onRecvCallback) { return mal_device_config_init_capture_ex(format, channels, sampleRate, NULL, onRecvCallback); }

// A simplified version of mal_device_config_init() for playback devices.
static MAL_INLINE mal_device_config mal_device_config_init_playback_ex(mal_format format, mal_uint32 channels, mal_uint32 sampleRate, mal_channel channelMap[MAL_MAX_CHANNELS], mal_send_proc onSendCallback) { return mal_device_config_init_ex(format, channels, sampleRate, channelMap, NULL, onSendCallback); }
static MAL_INLINE mal_device_config mal_device_config_init_playback(mal_format format, mal_uint32 channels, mal_uint32 sampleRate, mal_send_proc onSendCallback) { return mal_device_config_init_playback_ex(format, channels, sampleRate, NULL, onSendCallback); }



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utiltities
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Creates a mutex.
//
// A mutex must be created from a valid context. A mutex is initially unlocked.
mal_result mal_mutex_init(mal_context* pContext, mal_mutex* pMutex);

// Deletes a mutex.
void mal_mutex_uninit(mal_mutex* pMutex);

// Locks a mutex with an infinite timeout.
void mal_mutex_lock(mal_mutex* pMutex);

// Unlocks a mutex.
void mal_mutex_unlock(mal_mutex* pMutex);


// Retrieves a friendly name for a backend.
const char* mal_get_backend_name(mal_backend backend);

// Adjust buffer size based on a scaling factor.
//
// This just multiplies the base size by the scaling factor, making sure it's a size of at least 1.
mal_uint32 mal_scale_buffer_size(mal_uint32 baseBufferSize, float scale);

// Calculates a buffer size in milliseconds from the specified number of frames and sample rate.
mal_uint32 mal_calculate_buffer_size_in_milliseconds_from_frames(mal_uint32 bufferSizeInFrames, mal_uint32 sampleRate);

// Calculates a buffer size in frames from the specified number of milliseconds and sample rate.
mal_uint32 mal_calculate_buffer_size_in_frames_from_milliseconds(mal_uint32 bufferSizeInMilliseconds, mal_uint32 sampleRate);

// Retrieves the default buffer size in milliseconds based on the specified performance profile.
mal_uint32 mal_get_default_buffer_size_in_milliseconds(mal_performance_profile performanceProfile);

// Calculates a buffer size in frames for the specified performance profile and scale factor.
mal_uint32 mal_get_default_buffer_size_in_frames(mal_performance_profile performanceProfile, mal_uint32 sampleRate);

#endif  // MAL_NO_DEVICE_IO




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Decoding
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef MAL_NO_DECODING

typedef struct mal_decoder mal_decoder;

typedef enum
{
	mal_seek_origin_start,
	mal_seek_origin_current
} mal_seek_origin;

typedef size_t(*mal_decoder_read_proc)         (mal_decoder* pDecoder, void* pBufferOut, size_t bytesToRead); // Returns the number of bytes read.
typedef mal_bool32(*mal_decoder_seek_proc)         (mal_decoder* pDecoder, int byteOffset, mal_seek_origin origin);
typedef mal_result(*mal_decoder_seek_to_frame_proc)(mal_decoder* pDecoder, mal_uint64 frameIndex);
typedef mal_result(*mal_decoder_uninit_proc)       (mal_decoder* pDecoder);

typedef struct
{
	mal_format format;      // Set to 0 or mal_format_unknown to use the stream's internal format.
	mal_uint32 channels;    // Set to 0 to use the stream's internal channels.
	mal_uint32 sampleRate;  // Set to 0 to use the stream's internal sample rate.
	mal_channel channelMap[MAL_MAX_CHANNELS];
	mal_channel_mix_mode channelMixMode;
	mal_dither_mode ditherMode;
	mal_src_algorithm srcAlgorithm;
	union
	{
		mal_src_config_sinc sinc;
	} src;
} mal_decoder_config;

struct mal_decoder
{
	mal_decoder_read_proc onRead;
	mal_decoder_seek_proc onSeek;
	void* pUserData;
	mal_format  internalFormat;
	mal_uint32  internalChannels;
	mal_uint32  internalSampleRate;
	mal_channel internalChannelMap[MAL_MAX_CHANNELS];
	mal_format  outputFormat;
	mal_uint32  outputChannels;
	mal_uint32  outputSampleRate;
	mal_channel outputChannelMap[MAL_MAX_CHANNELS];
	mal_dsp dsp;                // <-- Format conversion is achieved by running frames through this.
	mal_decoder_seek_to_frame_proc onSeekToFrame;
	mal_decoder_uninit_proc onUninit;
	void* pInternalDecoder;     // <-- The drwav/drflac/stb_vorbis/etc. objects.
	struct
	{
		const mal_uint8* pData;
		size_t dataSize;
		size_t currentReadPos;
	} memory;   // Only used for decoders that were opened against a block of memory.
};

mal_decoder_config mal_decoder_config_init(mal_format outputFormat, mal_uint32 outputChannels, mal_uint32 outputSampleRate);

mal_result mal_decoder_init(mal_decoder_read_proc onRead, mal_decoder_seek_proc onSeek, void* pUserData, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_wav(mal_decoder_read_proc onRead, mal_decoder_seek_proc onSeek, void* pUserData, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_flac(mal_decoder_read_proc onRead, mal_decoder_seek_proc onSeek, void* pUserData, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_vorbis(mal_decoder_read_proc onRead, mal_decoder_seek_proc onSeek, void* pUserData, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_mp3(mal_decoder_read_proc onRead, mal_decoder_seek_proc onSeek, void* pUserData, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_raw(mal_decoder_read_proc onRead, mal_decoder_seek_proc onSeek, void* pUserData, const mal_decoder_config* pConfigIn, const mal_decoder_config* pConfigOut, mal_decoder* pDecoder);

mal_result mal_decoder_init_memory(const void* pData, size_t dataSize, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_memory_wav(const void* pData, size_t dataSize, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_memory_flac(const void* pData, size_t dataSize, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_memory_vorbis(const void* pData, size_t dataSize, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_memory_mp3(const void* pData, size_t dataSize, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_memory_raw(const void* pData, size_t dataSize, const mal_decoder_config* pConfigIn, const mal_decoder_config* pConfigOut, mal_decoder* pDecoder);

#ifndef MAL_NO_STDIO
mal_result mal_decoder_init_file(const char* pFilePath, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_file_wav(const char* pFilePath, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
#endif

mal_result mal_decoder_uninit(mal_decoder* pDecoder);

mal_uint64 mal_decoder_read(mal_decoder* pDecoder, mal_uint64 frameCount, void* pFramesOut);
mal_result mal_decoder_seek_to_frame(mal_decoder* pDecoder, mal_uint64 frameIndex);


// Helper for opening and decoding a file into a heap allocated block of memory. Free the returned pointer with mal_free(). On input,
// pConfig should be set to what you want. On output it will be set to what you got.
#ifndef MAL_NO_STDIO
mal_result mal_decode_file(const char* pFilePath, mal_decoder_config* pConfig, mal_uint64* pFrameCountOut, void** ppDataOut);
#endif
mal_result mal_decode_memory(const void* pData, size_t dataSize, mal_decoder_config* pConfig, mal_uint64* pFrameCountOut, void** ppDataOut);

#endif  // MAL_NO_DECODING


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Generation
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	double amplitude;
	double periodsPerSecond;
	double delta;
	double time;
} mal_sine_wave;

mal_result mal_sine_wave_init(double amplitude, double period, mal_uint32 sampleRate, mal_sine_wave* pSineWave);
mal_uint64 mal_sine_wave_read(mal_sine_wave* pSineWave, mal_uint64 count, float* pSamples);
mal_uint64 mal_sine_wave_read_ex(mal_sine_wave* pSineWave, mal_uint64 frameCount, mal_uint32 channels, mal_stream_layout layout, float** ppFrames);


#ifdef __cplusplus
}
#endif


#endif // mini_al_h__