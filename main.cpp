#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>


#include <SFML/Graphics.hpp>

#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <list>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundSource.hpp>
#include <SFML/Audio/SoundStream.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#pragma warning(disable:4996)

using namespace std;
using namespace sf;

int FPS = 60; // Maximum frames per second

int WINDOW_W = 600; // Window width
int WINDOW_H = 800; // Window height

// for values affected by FPS, use GLOBALdefine_name_in_small_letters
float GRAVITY; // affected by FPS
float V_ACCEL; // affected by FPS
float V_HOLD;
float CUT_V_VEL; // affected by FPS
float MAX_V_VEL; // affected by FPS

float GLOBALgravity;
float GLOBALv_accel;
float GLOBALcut_v_vel;
float GLOBALmax_v_vel;

// More Game Variables
bool isPlaying;		// Indicates whether player is still playing (true) or already in Game Over (false)
int score = 0;			// Keeps track of player score
float graceSpaceX;	// Horizontal grace space when checking for collision between bird and pipes
float graceSpaceY;	// Vertical grace space when checking for collision between bird and pipes

/**
 * Class representing any pipe entity.
 */
class Pipes
{
public:
	Sprite pipeUpper;
	Sprite pipeLower;

	Texture pipeUpperTex;
	Texture pipeLowerTex;

	RectangleShape pipeGap; // pipeGap is to keep track of the score by counting intersections with it and player
	bool scorePass = true;

	/**
	 * @brief Consturctor
	 */
	Pipes()
	{
		if (!pipeUpperTex.loadFromFile("assets/pipeUpper.png"))
		{
			cout << "pipeUpper sprite failed to load." << endl;
		}
		else { cout << "pipeUpper sprite successfully loaded!" << endl; };

		pipeUpperTex.setSmooth(false);
		pipeUpperTex.setRepeated(false);

		if (!pipeLowerTex.loadFromFile("assets/pipeLower.png"))
		{
			cout << "pipeLower sprite failed to load." << endl;
		}
		else { cout << "pipeLower sprite successfully loaded!" << endl; };

		pipeLowerTex.setSmooth(false);
		pipeLowerTex.setRepeated(false);

		pipeUpper.setTexture(pipeUpperTex);
		pipeLower.setTexture(pipeLowerTex);

		pipeGap.setSize(Vector2f(100, 200));
		pipeGap.setFillColor(Color::Transparent);

		// FOR DEBUGGING
		//pipeGap.setOutlineColor(Color::Yellow);
		//pipeGap.setOutlineThickness(2);

		// make origin be the center
		pipeUpper.setOrigin(50, 250);
		pipeGap.setOrigin(50, 100);
		pipeLower.setOrigin(50, 250);
	}

	/**
	 * @brief Draws this pipe entity
	 * @param[in] window Window where this entity will be drawn
	 */
	void draw(sf::RenderWindow& window)
	{
		window.draw(pipeUpper);
		window.draw(pipeLower);
		window.draw(pipeGap);
	}

	/**
	 * @brief Set the position of the pipes
	 * @param[in] x X-position
	 * @param[in] y Y-position
	 */
	void setPipePosition(float x, float y)
	{
		pipeUpper.setPosition(x, y);

		// set position such that every other rectangle shape will be affected by pipeUpper's position
		pipeGap.setPosition(pipeUpper.getPosition().x, pipeUpper.getPosition().y + 350);
		pipeLower.setPosition(pipeUpper.getPosition().x, pipeUpper.getPosition().y + 700);
	}

	/**
	 * @brief Return the x position of the pipes
	 */
	float getPipePositionX()
	{
		return pipeUpper.getPosition().x;
	}

	/**
	 * @brief Return the y position of the pipes
	 */
	float getPipePositionY()
	{
		return pipeUpper.getPosition().y;
	}

};

/**
 * Give a random number of the Y position of the pipes
 */
float randPosY()
{
	return -150 + rand() % 400; // offset of 200 from either direction from the center
}

/**
 * Class representing BIRD
 */

class Bird
{
public:
	Sprite bird;
	Texture birdTexture;
	int jumpNumFrames;  // number of frames remaining until we don't apply jump acceleration anymore
	float vAccel;   // current vertical acceleration
	float vVel;     // current vertical velocity

	/**
	 * @brief Constructor
	 */
	Bird()
	{
		// assume player is grounded at the beginning, hence the initialized values
		jumpNumFrames = V_HOLD;
		vAccel = 0;
		vVel = 0;

		//Load bird texture
		if (!birdTexture.loadFromFile("assets/bird.png"))
		{
			cout << "bird sprite failed to load." << endl;
		}
		else { cout << "bird sprite successfully loaded!" << endl; };

		birdTexture.setSmooth(false);
		birdTexture.setRepeated(false);

		bird.setTexture(birdTexture);

		// make origin be the center
		bird.setOrigin(25, 25);

	}

	/**
	 * @brief Draws the bird
	 * @param[in] window Window (where entity will be drawn)
	 */
	void draw(sf::RenderWindow& window)
	{
		window.draw(bird);
	}

	/**
	 * @brief Sets the position of the bird.
	 * @param[in] x X-position
	 * @param[in] y Y-position
	 * @note The position is based on the center due to how setSize is written
	 */
	void setPosition(float x, float y)
	{
		bird.setPosition(x, y);
	}

	/**
	 * @brief Process vertical movement
	 */
	void vMove()
	{
		// This is because the bird is always experiencing gravity
		vAccel = GLOBALgravity;

		// On Jump (Space for jump)
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{

			if (jumpNumFrames > 0)
			{
				vAccel = GLOBALv_accel;
			}

			jumpNumFrames -= 1; // After jumping, decrease the number of frames where Globalv_accel is applied

		}
		else
		{
			// Free Fall (Bird continues to experience gravity

			// However, if Globalcut_v_vel is reached, keep vVel = GLOBALcut_v_vel
			if (vVel < GLOBALcut_v_vel)
			{
				vVel = GLOBALcut_v_vel;
			}

			//Refresh the number of jumpNumFrames when jump key is not pressed
			jumpNumFrames = V_HOLD;

		}

		// jumpNumFrames can't be less than 0
		if (jumpNumFrames < 0) {

			jumpNumFrames = 0;

		}

		// Apply vertical acceleration to vertical velocity
		vVel += vAccel;

		// Maximum falling velocitycannot go beyond GLOBALmax_v_vel
		if (vVel > GLOBALmax_v_vel)
		{
			vVel = GLOBALmax_v_vel;
		}

		// move bird based on vertical velocity
		bird.move(0, vVel);
	}

};

int main()
{
	// Create an SFML window of size 800x800 with an appropriate title
	sf::RenderWindow window(sf::VideoMode(WINDOW_W, WINDOW_H), "Flappy Bird");

	// We tell SFML to limit our application to whatever FPS' value is
	window.setFramerateLimit(FPS);

	// Load sound fx for scoring
	SoundBuffer scoreBuffer;
	scoreBuffer.loadFromFile("fx/Score.wav");
	Sound ding;
	ding.setBuffer(scoreBuffer);
	ding.setVolume(120.f);

	// Load sound fx for game over
	SoundBuffer gameoverBuffer;
	gameoverBuffer.loadFromFile("fx/Gameover.wav");
	Sound smack;
	smack.setBuffer(gameoverBuffer);
	smack.setVolume(120.f);

	// game background
	Sprite background;
	Texture backgroundTex;

	if (!backgroundTex.loadFromFile("assets/background.png"))
	{
		cout << "background sprite failed to load." << endl;
	}
	else { cout << "background sprite successfully loaded!" << endl; };

	backgroundTex.setSmooth(false);
	backgroundTex.setRepeated(false);

	background.setTexture(backgroundTex);

	// 3 pipes so that there will only be 2 pipes present onscreen at a time
	Pipes pipePairs[3];

	// initially set the pipes to be beyond the screen to the right
	for (int i = 0; i < 3; i++)
	{
		// pipes always have a distance of 200 pixels between each other
		pipePairs[i].setPipePosition(650 + i * 300, randPosY());
	}

	// speed at which the pipes move (should increase as points increase)
	float speed = 1.5;


	// Initializing Bird
	Bird bird;
	bird.setPosition(100, 100);

	GRAVITY = 20;
	V_ACCEL = -600;
	V_HOLD = 1;
	CUT_V_VEL = -20;
	MAX_V_VEL = 400;

	GLOBALgravity = GRAVITY / FPS;
	GLOBALv_accel = V_ACCEL / FPS;
	GLOBALcut_v_vel = CUT_V_VEL / FPS;
	GLOBALmax_v_vel = MAX_V_VEL / FPS;

	// Initializing Game Variables
	isPlaying = true;
	score = 0;
	graceSpaceX = 10;
	graceSpaceY = 10;

	/*Sprite background;
	Texture backgroundTex;

	if (!backgroundTex.loadFromFile("assets/background.png"))
	{
		cout << "background sprite failed to load." << endl;
	}
	else { cout << "background sprite successfully loaded!" << endl; };

	backgroundTex.setSmooth(false);
	backgroundTex.setRepeated(false);

	background.setTexture(backgroundTex);*/

	// start screen
	Sprite getReady;
	Texture getReadyTex;

	if (!getReadyTex.loadFromFile("assets/Get Ready_text.png")) {
		cout << "getReady failed to load." << endl;
	}
	else {
		cout << "getReady succesfully loaded!" << endl;
	}

	getReadyTex.setSmooth(false);
	getReadyTex.setRepeated(false);
	getReady.setTexture(getReadyTex);
	getReady.setOrigin(getReady.getLocalBounds().width / 2, getReady.getLocalBounds().height / 2);
	getReady.setPosition(window.getSize().x / 2, 80);

	Sprite guide;
	Texture guideTex;

	if (!guideTex.loadFromFile("assets/Space_tutorial.png")) {
		cout << "Guide failed to load." << endl;
	}
	else {
		cout << "Guide succesfully loaded!" << endl;
	}

	guideTex.setSmooth(false);
	guideTex.setRepeated(false);
	guide.setTexture(guideTex);
	guide.setOrigin(guide.getLocalBounds().width / 2, guide.getLocalBounds().height / 2);
	guide.setPosition(window.getSize().x / 2, 640);
	
	// game over screen
	Sprite gameover;
	Texture gameoverTex;

	if (!gameoverTex.loadFromFile("assets/Game Over_text.png")) {
		cout << "Gameover failed to load." << endl;
	}
	else {
		cout << "Gameover succesfully loaded!" << endl;
	}

	gameoverTex.setSmooth(false);
	gameoverTex.setRepeated(false);
	gameover.setTexture(gameoverTex);
	gameover.setOrigin(gameover.getLocalBounds().width / 2, gameover.getLocalBounds().height / 2);
	gameover.setPosition(window.getSize().x / 2, window.getSize().y / 2);

	// text and font for scoring
	Text scoreText;
	Font scoreFont;
	if (!scoreFont.loadFromFile("assets/Minecraftia-Regular.ttf")) {
		cout << "scoreFont failed to load." << endl;
	}
	else {
		cout << "scoreFont succesfully loaded!" << endl;
	}
	scoreText.setFont(scoreFont);
	scoreText.setFillColor(sf::Color::White);
	scoreText.setOutlineColor(sf::Color(84, 56, 71));
	scoreText.setOutlineThickness(6);
	scoreText.setCharacterSize(64);
	scoreText.setOrigin(scoreText.getLocalBounds().width / 2, scoreText.getLocalBounds().height / 2);
	scoreText.setPosition(window.getSize().x / 2, 640);

	// Instead of an explicit "isPlaying" bool variable,
	// we check if the window is still open or not to signify
	// whether we are still playing or not.
	// We use the SFML window's isOpen() function, which returns a bool value,
	// to check whether the window is open or not.
	while (window.isOpen())
	{
		// --- Clear screen ---

		// Clear the screen using the SFML window's clear() function.
		// By default, SFML clears the screen by replacing all the colors 
		// in the background with black.
		window.clear();

		// Alternatively, we can provide a color to clear the screen with.
		// ex. window.clear(sf::Color::Red);

		// --- Process input/events ---


		// We create a variable to store information about 
		// events that happen to our window, e.g., window closed, key pressed, etc.
		sf::Event event;

		// We now check if there were events triggered by the window since the last
		// iteration of the loop, and react to them accordingly.
		// As mentioned in the slides, we can also directly update the current state 
		// at this point, e.g., change player position when a keyboard input event happened
		while (window.pollEvent(event))
		{
			// We can use else if's to handle other types of events like
			// keyboard input, mouse button click, etc.
			// else if (event.type == ...)
			// 
			// Alternatively, you can use a switch statement instead of an if-else-if chain
		}

		// Close the game
		if (sf::Keyboard::isKeyPressed(Keyboard::Escape))
		{
			window.close();
		}

		// Gameplay
		if (isPlaying) {

			// --- Update/advance state ---
			// makes the pipes move leftwards
			for (int i = 0; i < 3; i++)
			{
				pipePairs[i].setPipePosition(pipePairs[i].getPipePositionX() - speed, pipePairs[i].getPipePositionY());

				// makes the pipe loop back to the right side of the screen (with a new y position)
				if (pipePairs[i].getPipePositionX() <= -250)
				{
					pipePairs[i].setPipePosition(650, randPosY());

					// Reset pipePair to allow scoring again
					pipePairs[i].scorePass = true;
				}
			}

			//MOVE BIRD
			bird.vMove();

			// COLLISION CHECKS BETWEEN BIRD AND PIPES
			for (int i = 0; i < 3; i++)
			{
				// Check for collision with pipeUpper and pipeLower
				// if there is a collision, set stillPlaying to false
				sf::Vector2f birdMax = bird.bird.getPosition() + bird.bird.getOrigin() + Vector2f(graceSpaceX, graceSpaceY);
				sf::Vector2f birdMin = bird.bird.getPosition() - bird.bird.getOrigin() - Vector2f(graceSpaceX, graceSpaceY);

				sf::Vector2f upperMax = pipePairs[i].pipeUpper.getPosition() + pipePairs[i].pipeUpper.getOrigin();
				sf::Vector2f upperMin = pipePairs[i].pipeUpper.getPosition() - pipePairs[i].pipeUpper.getOrigin();

				sf::Vector2f lowerMax = pipePairs[i].pipeLower.getPosition() + pipePairs[i].pipeLower.getOrigin();
				sf::Vector2f lowerMin = pipePairs[i].pipeLower.getPosition() - pipePairs[i].pipeLower.getOrigin();

				sf::Vector2f gapMax = pipePairs[i].pipeGap.getPosition() + pipePairs[i].pipeGap.getOrigin();
				sf::Vector2f gapMin = pipePairs[i].pipeGap.getPosition() - pipePairs[i].pipeGap.getOrigin();

				bool upperCollisionCheck =
					(birdMin.x < upperMax.x) && (birdMin.y < upperMax.y) &&
					(upperMin.x < birdMax.x) && (upperMin.y < birdMax.y);

				bool lowerCollisionCheck =
					(birdMin.x < lowerMax.x) && (birdMin.y < lowerMax.y) &&
					(lowerMin.x < birdMax.x) && (lowerMin.y < birdMax.y);

				bool gapCollisionCheck =
					(birdMin.x < gapMax.x) && (birdMin.y < gapMax.y) &&
					(gapMin.x < birdMax.x) && (gapMin.y < birdMax.y);

				// If bird collides with pipePairs
				// Go to end screen
				if (upperCollisionCheck || lowerCollisionCheck) {
					isPlaying = false;
					cout << "HIT" << endl;
					smack.play();
				}

				// If bird collides with pipeGap
				// Add to score and speed, and set scorePass to false
				if (gapCollisionCheck && pipePairs[i].scorePass) {
					cout << ++score << endl;
					speed += 0.25;
					pipePairs[i].scorePass = false;
					ding.play();
				}
			}

			// Render background, pipes and bird
			window.draw(background);
			bird.draw(window);
			for (Pipes pipe : pipePairs)
			{
				pipe.draw(window);
			}

			// Update the score
			stringstream scoreString;
			scoreString << score;
			scoreText.setString(scoreString.str());

			// Show score when the game starts
			if (score > 0) {
				window.draw(scoreText);
			}

			// Title screen
			if (score < 1) {
				window.draw(getReady);
				window.draw(guide);
			}
		}

		if (!isPlaying) {
			// Render background & pipes
			window.draw(background);
			for (Pipes pipe : pipePairs)
			{
				pipe.draw(window);
			}

			// Game over screen
			window.draw(gameover);
		}
		

		// Finally, tell SFML to render/display whatever we drew to the screen
		window.display();
	}
}
