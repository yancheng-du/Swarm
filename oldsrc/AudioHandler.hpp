#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include <vector>
#include <math.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

static void list_audio_devices(const ALCchar *devices)
{
	const ALCchar *device = devices, *next = devices + 1;
	size_t len = 0;

	fprintf(stdout, "Devices list:\n");
	fprintf(stdout, "----------\n");
	while (device && *device != '\0' && next && *next != '\0') {
		fprintf(stdout, "%s\n", device);
		len = strlen(device);
		device += (len + 1);
		next += (len + 2);
	}
	fprintf(stdout, "----------\n");
}

#define TEST_ERROR(_msg)		\
	error = alGetError();		\
	if (error == AL_OUT_OF_MEMORY ) {	\
		printf("AL_OUT_OF_MEMORY");	\
	\
	}				\
	if (error == AL_INVALID_VALUE ) {	\
		printf("AL_INVALID_VALUE");	\
		\
	}				\
	if (error == AL_INVALID_ENUM ) {	\
		printf("AL_INVALID_ENUM");	\
	\
	}				\
	if (error != AL_NO_ERROR) {	\
		fprintf(stderr, _msg "\n");	\
	\
	}

class AudioHandler{

private:
	ALboolean enumeration;
	const ALCchar *devices;
	const ALCchar *defaultDeviceName;
	int ret;
//	int sound_bees;
	int buffnum;
	char *bufferData;
	ALCdevice *device;
	ALvoid *data;
	ALCcontext *context;
	ALsizei size, freq;
	ALenum format;
	ALuint* buffer;
	ALuint* source;
	ALboolean loop = AL_FALSE;
	ALCenum error;
	ALint source_state;

public:
	AudioHandler(int width, int height){
		buffnum = 33;
//		sound_bees = num_sound_bees;
//	source = (ALuint*)malloc(sizeof(ALuint)*sound_bees);
		buffer = (ALuint*)malloc(sizeof(ALuint)*buffnum);
		source = (ALuint*)malloc(sizeof(ALuint)*buffnum);
		ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
		enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
		if (enumeration == AL_FALSE)
			fprintf(stderr, "enumeration extension not available\n");

		alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);

		list_audio_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));

		defaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

		device = alcOpenDevice(defaultDeviceName);
		if (!device) {
			fprintf(stderr, "unable to open default device\n");
		}

		fprintf(stdout, "Device: %s\n", alcGetString(device, ALC_DEVICE_SPECIFIER));

		alGetError();

		context = alcCreateContext(device, NULL);
		if (!alcMakeContextCurrent(context)) {
			fprintf(stderr, "failed to make default context\n");
		}
		TEST_ERROR("make default context");

		/* set orientation */
		alListener3f(AL_POSITION, width/2, height/2, 1.0f);
		TEST_ERROR("listener position");
		alListenerf(AL_GAIN, 0.8 );
		TEST_ERROR("listener gain");
		alListener3f(AL_VELOCITY, 0, 0, 0);
		TEST_ERROR("listener velocity");
		alListenerfv(AL_ORIENTATION, listenerOri);
		TEST_ERROR("listener orientation");

		alGenSources((ALuint)buffnum, source);
		TEST_ERROR("source generation");

		for(int i=0;i<buffnum;i++){

			float gain = 1.0;
			if(i >=9  && i <= 16)
				gain = 0.7;

			if(i >= 17 && i <= 24)
				gain = 0.8;

			alSourcef(source[i], AL_PITCH, 1);
			TEST_ERROR("source pitch");
			alSourcef(source[i], AL_GAIN, gain);
			TEST_ERROR("source gain");
			alSource3f(source[i], AL_POSITION, width/2, height/2, 1.0f);
			TEST_ERROR("source position");
			alSource3f(source[i], AL_VELOCITY, 0, 0, 0);
			TEST_ERROR("source velocity");
			alSourcei(source[i], AL_MAX_DISTANCE, 400);
			TEST_ERROR("max distance");
			alSourcei(source[i], AL_LOOPING, AL_FALSE);
			TEST_ERROR("source looping");
		}
		alSourcei(source[buffnum-1], AL_LOOPING, AL_TRUE);
		TEST_ERROR("source looping");

		alGenBuffers((ALuint)buffnum, buffer);
		TEST_ERROR("buffer generation");

		//load new .wavs
		for(int i=1;i<=buffnum;i++){
			string str = to_string(i);
			string flnm = ".wav";
			string flhdr = "./sounds/";
			string file_name = flhdr+str+flnm;
			alutLoadWAVFile((ALbyte*)file_name.c_str(), &format, &data, &size, &freq, (ALboolean*)&loop);
			TEST_ERROR("loading wav file");

			alBufferData(buffer[i-1], format, data, size, freq);
			TEST_ERROR("buffer copy");
		}

		for(int i=0;i<buffnum;i++){
			alSourcei(source[i], AL_BUFFER, buffer[i]);
			TEST_ERROR("buffer binding");
		}

	}

	void play_sound(int i){
		alGetSourcei(source[i], AL_SOURCE_STATE, &source_state);
		TEST_ERROR("source state get");
		if(source_state != AL_PLAYING) {
			alSourcePlay(source[i]);
		}
	}
	void set_point(int i, int x, int y){
		alSource3f(source[i], AL_POSITION, x, y, 1.0f);
		TEST_ERROR("source position");
	}

	void delete_sources(){
		alDeleteSources(buffnum, source);
		alDeleteBuffers(buffnum, buffer);
		device = alcGetContextsDevice(context);
		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		alcCloseDevice(device);
	}

};
