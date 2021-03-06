#pragma once
#ifndef WS_AUDIO_SOURCE_H
#define WS_AUDIO_SOURCE_H
#include "Component.h"
#include<SFML\Audio\Sound.hpp>
namespace WeiSungEngine {
	class AudioSource : public Component {
	public:
		AudioSource();
		AudioSource(std::weak_ptr<GameObject> gameObject);
		~AudioSource();
		const ComponentType Type() const override { return COMPONENT_AUDIO_SOURCE; }
		void SetEnabled(const bool & enabled) override;
		void Init(const std::string & path, const bool & play = false, const bool & loop = false);
		const std::string GetName() const override { return "AudioSource"; }
		void SetVolume(const float & volume);
		float GetVolume();
		void SetAttenuation(const float & maxDistance, const float & attenuation);
		void SetPosition(const Vector2 & position);
		float GetAttenuation();
		void SetLooping(const bool & looping);
		bool GetLooping();
		bool IsPlaying();
		bool IsPaused();
		void Play();
		void Stop();
	private:
		sf::Sound sound;
	};
}
#endif // !WS_AUDIO_SOURCE_H
