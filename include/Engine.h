
#ifndef CEREAL_XML_STRING_VALUE
#define CEREAL_XML_STRING_VALUE "WaiSungSerializedData"
#endif // CEREAL_XML_STRING_VALUE
#ifndef WS_ENGINE_H
#define WS_ENGINE_H
#include <memory>
namespace WeiSungEngine {
	class Graphics;
	class Input;
	class Timer;
	class FileIO;
	class Game;
	class DebugDraw;
	class Engine {
	public:

		~Engine();
		static Engine & GetInstance();
		void Start();
		void Run();
		void End();
		void Close();
		bool IsRunning();
		void SetGame(std::shared_ptr<Game> newGame);
		std::weak_ptr<Graphics> GetGraphics();
		DebugDraw * GetDebugDraw();
		std::weak_ptr<Input> GetInput();
		std::weak_ptr<Timer> GetTimer();
		unsigned int GetFPS();

	private:
		Engine();
		std::shared_ptr<Graphics> graphics;
		std::shared_ptr<Input> input;
		std::shared_ptr<Timer> timer;
		std::shared_ptr<Game> game;
		DebugDraw * debugDraw;
		bool running;
		unsigned int fps = 0, framesThisSecond = 0;
		float fpsTimer = 0.0f;
	};
}

#endif // !WS_ENGINE_H
