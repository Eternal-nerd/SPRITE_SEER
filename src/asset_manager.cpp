#include "asset_manager.h"

/*
-----~~~~~=====<<<<<{_INITIALIZATION_}>>>>>=====~~~~~-----
*/
void AssetManager::init(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue) {
    log(name_ + __func__, "initializing asset manager");

    physicalDevice_ = physicalDevice;
    device_ = device;
    commandPool_ = commandPool;
    graphicsQueue_ = graphicsQueue;

    // iterate through resource directory, grabbing all image and audio files
	enumerateFiles();
	initTextures();
	initAudio();
}

void AssetManager::enumerateFiles() {
	log(name_ + __func__, "enumerating asset filenames");

	fs::path toCheck = "../res";

	if (!fs::is_directory(toCheck)) {
		throw std::runtime_error("could not find the res/ directory");
	}

	for (auto const& entry : fs::recursive_directory_iterator(toCheck)) {
		if (entry.path().has_extension()) {
			log(name_ + __func__, "found file: " + entry.path().generic_string());
			fs::path ext = entry.path().extension();
			if (ext == ".jpg" || ext == ".png") {
				textureFilenames_.push_back(entry.path().generic_string());
			}
			if (ext == ".wav") {
				audioFilenames_.push_back(entry.path().generic_string());
			}
		}
	}
}

void AssetManager::initTextures() {
	log(name_ + __func__, "creating textures");
	for (int i = 0; i < textureFilenames_.size(); i++) {
		Texture t;
		t.create(textureFilenames_[i], physicalDevice_, device_, commandPool_, graphicsQueue_);
		textures_.push_back(t);
	}
}

void AssetManager::initAudio() {
	log(name_ + __func__, "initializing SDL audio");

	// Audio -------------------------------------------====================<
	// for now, create SDL audio stream on init, and set audio spec to first file in list..
	if (audioFilenames_.size() < 1) {
		throw std::runtime_error("need to include atleast 1 audio file (WAV)");
	}
	if (!SDL_LoadWAV(audioFilenames_[0].c_str(), &audioSpec_, &wavData_, &wavDataLen_)) {
		throw std::runtime_error("failed to load .WAV file: " + audioFilenames_[0]);
	}

	// Create our audio stream in the same format as the .wav file. It'll convert to what the audio hardware wants.
	stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec_, NULL, NULL);
	if (!stream_) {
		SDL_Log("Couldn't create audio stream: %s", SDL_GetError());
		throw std::runtime_error("Failed to create SDL audio stream! ");
	}

	// lower volume
	SDL_SetAudioStreamGain(stream_, 0.1f);

	// SDL_OpenAudioDeviceStream starts the device paused. You have to tell it to start!
	SDL_ResumeAudioStreamDevice(stream_);
}

/*
-----~~~~~=====<<<<<{_GETTERS_}>>>>>=====~~~~~-----
*/
int AssetManager::getTextureCount() { return static_cast<int>(textures_.size()); }
const VkImageView& AssetManager::getTextureImageView(int index) const { return textures_[index].getImageView(); }
const VkSampler& AssetManager::getTextureImageSampler(int index) const { return textures_[index].getSampler(); }

int AssetManager::getTextureIndex(const std::string& filename) {
	for (int i = 0; i < textureFilenames_.size(); i++) {
		if (filename == textureFilenames_[i]) {
			return i;
		}
	}
	throw std::runtime_error("failed to find texture: " + filename);
}

/*
-----~~~~~=====<<<<<{_SOUNDS_}>>>>>=====~~~~~-----
*/
void AssetManager::playSound(const std::string& filename) {
	// get sound index
	int soundIndex = -1;
	for (int i = 0; i < audioFilenames_.size(); i++) {
		if (audioFilenames_[i] == filename) {
			soundIndex = i;
			break;
		}
	}

	if (soundIndex == -1) {
		throw std::runtime_error("failed to find/play audio filename: " + filename);
	}

	// free PCM data previously loaded into buffer
	SDL_free(wavData_);

	if (!SDL_LoadWAV(audioFilenames_[soundIndex].c_str(), &audioSpec_, &wavData_, &wavDataLen_)) {
		throw std::runtime_error("Failed to load .WAV file: " + audioFilenames_[soundIndex]);
	}

	// clear audiostream incase of past sounds still replaying:
	// FIXMEEEEE need to add overlapping sounds
	SDL_ClearAudioStream(stream_);

	// feed more data to the stream. It will queue at the end, and trickle out as the hardware needs more data. 
	SDL_PutAudioStreamData(stream_, wavData_, wavDataLen_);
}

/*
-----~~~~~=====<<<<<{_CLEANUP_}>>>>>=====~~~~~-----
*/
void AssetManager::cleanup() {
	log(name_ + __func__, "cleaning up");

	// texture cleanup here
	for (int i = 0; i < textures_.size(); i++) {
		textures_[i].destroy();
	}

	log(name_ + __func__, "destroying audio object");
	SDL_free(wavData_);
}
