#include "Graphics.h"
#include "TypeConversion.h"
#include <iostream>

static  bool operator == (sf::ContextSettings lhs, sf::ContextSettings rhs) {
	return lhs.antialiasingLevel == rhs.antialiasingLevel && lhs.attributeFlags == rhs.attributeFlags && lhs.depthBits == rhs.depthBits && lhs.majorVersion == rhs.majorVersion && lhs.stencilBits == rhs.stencilBits;
}

static bool operator != (sf::ContextSettings lhs, sf::ContextSettings rhs) {
	return !(lhs == rhs);
}

sf::Color getSFColour(const float & r, const float & g, const float & b, const float & a) {
	return sf::Color((uint8_t)(r * 255), (uint8_t)(g * 255), (uint8_t)(b * 255), (uint8_t)(a * 255));
}

Graphics::Graphics() {
	RebuildDisplay();
	if(!font.loadFromFile("arial.ttf")) {
		std::cout << "Couldn't load font" << std::endl;
	}
}

Graphics::~Graphics() {
}

void Graphics::Update() {
	sf::Event sfEvent;
	while(window.pollEvent(sfEvent)) {
		switch(sfEvent.type) {
		case sf::Event::Closed:
			window.close();
			windowOpen = false;
			break;
		case sf::Event::Resized:
			window.setSize(sf::Vector2u(sfEvent.size.width, sfEvent.size.height));
			break;
		default:
			break;
		}
	}


	window.display();
}

void Graphics::End() {
}

void Graphics::SetWindowTitle(const std::string & title) {
	settings.windowTitle = title;
	window.setTitle(title);
}

std::string Graphics::GetWindowTitle() {
	return settings.windowTitle;
}


void Graphics::SetScreenResolution(const float & width, const float & height) {
	settings.resolution.x = width; settings.resolution.y = height;
	sf::Vector2u screenSize = window.getSize();
	if(screenSize.x != width || screenSize.y != height) {
		window.setSize(sf::Vector2u((size_t)width, (size_t)height));
		RebuildDisplay();
	}
}

Vector2 Graphics::GetScreenResolution() {
	return settings.resolution;
}

float Graphics::GetScreenX() {
	return GetScreenPosition().x;
}

float Graphics::GetScreenY() {
	return GetScreenPosition().y;
}

inline void Graphics::SetScreenPosition(const float & x, const float & y) {
	if(settings.screenPosition.x != x || settings.screenPosition.y != y) {
		settings.screenPosition.x = x; settings.screenPosition.y = y;
		window.setPosition(sf::Vector2i((int)x, (int)y));
		settings.screenPosition = TypeConversion::ConvertToVector2(window.getPosition());
	}
}

Vector2 Graphics::GetScreenPosition() {
	settings.screenPosition = TypeConversion::ConvertToVector2(window.getPosition());
	return settings.screenPosition + Vector2((float)(GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER)),
											 (float)(GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXPADDEDBORDER)));
}

void Graphics::Clear() {
	window.clear();
}

void Graphics::Draw(sf::Sprite sprite) {
	window.draw(sprite);
}

void Graphics::Draw(const sf::Shape & shape) {
	window.draw(shape);
}

void Graphics::Draw(sf::VertexArray vertexArray) {
	window.draw(vertexArray);
}


//void GraphicsSFML::Draw(const std::string & text, const Vector2 & position) {
//	sf::Text toDraw;
//	toDraw.setFont(font);
//	toDraw.setString(text);
//	toDraw.setPosition(position.x, settings.resolution.y - position.y);
//	toDraw.setStyle(sf::Text::Bold);
//	toDraw.setFillColor(sf::Color::White);
//	window.draw(toDraw);
//}

void Graphics::Draw(const std::string & text, const Vector2 & position, unsigned int characterSize, TextAlignment alignment) {
	sf::Text toDraw;
	toDraw.setFont(font);
	toDraw.setString(text);
	toDraw.setCharacterSize(characterSize);
	toDraw.setStyle(sf::Text::Bold);
	if(alignment != TextAlignment::LEFT_ALIGNED) {
		float width = toDraw.getLocalBounds().width;
		if(alignment == TextAlignment::CENTRE_ALIGNED) toDraw.setOrigin(sf::Vector2f(width / 2, 0));
		else if(alignment == TextAlignment::RIGHT_ALIGNED) toDraw.setOrigin(sf::Vector2f(width, 0));
	}
	toDraw.setPosition(position.x, position.y);
	toDraw.setFillColor(sf::Color::White);

	window.draw(toDraw);
}

void Graphics::Draw(const sf::Drawable & d, const sf::Transform & t) {
	window.draw(d, t);
}

void Graphics::DrawCircle(const sf::CircleShape & c) {
	window.draw(c);
}

void Graphics::DrawCircle(const Vector2 & position, const float & radius, bool filled, const float & r, const float & g, const float & b, const float & a, const unsigned int & noOfVerts) {
	sf::CircleShape circle = sf::CircleShape(radius, noOfVerts);
	sf::Color colour = getSFColour(r, g, b, a);
	circle.setOutlineColor(colour);
	circle.setOutlineThickness(1);
	circle.setFillColor(filled ? colour : sf::Color::Transparent);
	circle.setOrigin(radius, radius);
	circle.setPosition(position.x, position.y);

	window.draw(circle);
}

void Graphics::DrawRect(const sf::RectangleShape & r) {
	window.draw(r);
}

void Graphics::DrawRect(const Vector2 & position, const Vector2 & size, const float & rotation, bool filled, const float & r, const float & g, const float & b, const float & a) {
	sf::RectangleShape rect(sf::Vector2f(size.x, size.y));
	sf::Color colour = getSFColour(r, g, b, a);
	if(rotation != 0.0f) rect.rotate(rotation);
	rect.setOutlineColor(colour);
	rect.setFillColor(filled ? colour : sf::Color::Transparent);
	rect.setOutlineThickness(1);
	rect.setOrigin(sf::Vector2f(size.x / 2, size.y / 2));
	rect.setPosition(position.x, position.y);
	window.draw(rect);
}

void Graphics::DrawLine(sf::Vertex line []) {
	window.draw(line, 2, sf::Lines);
}

void Graphics::SetDepth(const unsigned int & depth) {
	settings.depthBits = depth;
	if(sfSettings.depthBits != depth) {
		sfSettings.depthBits = depth;
		RebuildDisplay();
	}
}

unsigned int Graphics::GetDepth() {
	return settings.depthBits;
}

void Graphics::SetAntialiasingLevel(const unsigned int & antiAliasingLevel) {
	settings.antialiasingLevel = antiAliasingLevel;
	if(sfSettings.antialiasingLevel != antiAliasingLevel) {
		sfSettings.antialiasingLevel = antiAliasingLevel;
		RebuildDisplay();
	}
}

unsigned int Graphics::GetAntialiasingLevel() {
	return settings.antialiasingLevel;
}

void Graphics::SetFullScreen(const bool & fullScreen) {
	if(settings.fullScreen != fullScreen) {
		settings.fullScreen = fullScreen;
		RebuildDisplay();
	}
}

bool Graphics::GetFullScreen() {
	return settings.fullScreen;
}

void Graphics::SetResizeable(const bool & resizeable) {
	if(settings.resizeable != resizeable) {
		settings.resizeable = resizeable;
		if(!settings.fullScreen) {
			RebuildDisplay();
		}
	}
}

bool Graphics::GetResizeable() {
	return settings.resizeable;
}

void Graphics::SetVSync(const bool & enabled) {
	if(settings.vSync != enabled) {
		settings.vSync = enabled;
		window.setVerticalSyncEnabled(enabled);
	}
}

bool Graphics::GetVSync() {
	return settings.vSync;
}

void Graphics::SetMaxFPS(const unsigned int & maxFPS) {
	if(settings.maxFPS != maxFPS) {
		settings.maxFPS = maxFPS;
		window.setFramerateLimit(maxFPS);
	}
}

unsigned int Graphics::GetMaxFPS() {
	return settings.maxFPS;
}

GraphicsSettings Graphics::GetSettings() {
	return settings;
}



void Graphics::SetSettings(GraphicsSettings settings) {
	if(this->settings != settings) {
		this->settings = settings;
		sfSettings = TypeConversion::ConvertToSFContext(settings);
		RebuildDisplay();
	}
}

sf::RenderWindow & Graphics::GetWindow() {
	return window;
}

HWND Graphics::GetWindowHandle() {
	return window.getSystemHandle();
}

bool Graphics::GetWindowOpen() {
	return windowOpen;
}

void Graphics::RebuildDisplay() {
	window.create(sf::VideoMode((int)settings.resolution.x, (int)settings.resolution.y), settings.windowTitle,
				  (settings.fullScreen ? sf::Style::Fullscreen : (settings.resizeable ? sf::Style::Default : (sf::Style::Titlebar | sf::Style::Close))),
				  sfSettings);
	window.setPosition(sf::Vector2i((int)settings.screenPosition.x, (int)settings.screenPosition.y));
	view = window.getView();

	if(settings.vSync) window.setVerticalSyncEnabled(true);
	if(!windowOpen) windowOpen = true;
}

