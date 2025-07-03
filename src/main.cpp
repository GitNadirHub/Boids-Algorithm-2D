#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <random>
#include <thread>
using namespace sf;

bool wrap = true;

const float NEIGHBOUR_RADIUS = 25.F;

float separationWeight = 1.5f;
float alignmentWeight = 1.0f;
float cohesionWeight = 0.5f;

struct Boid {
    Vector2f position;
    Vector2f velocity = { 20.f, 20.f };
    float maxSpeed = 300.f;
	float minSpeed = 100.f;

    Boid(float x, float y)
    {
		position = Vector2f(x, y);
    }

    Boid()
    {

    }

    void update(float deltaTime);

    void draw(RenderWindow& window)
    {
        CircleShape shape(2.f);
        shape.setPosition(position);
        shape.setFillColor(Color::Green);
        window.draw(shape);
    }
};

Clock deltaClock;

Boid boids[5000];

int boidsCnt = 1000;

bool isAlmostColliding(const Boid &a, const Boid& b)
{

    float dx = a.position.x - b.position.x;
    float dy = a.position.y - b.position.y;

    if (dx * dx + dy * dy >= NEIGHBOUR_RADIUS * NEIGHBOUR_RADIUS) //if not close enough to collide
        return false;

	Vector2f distance = a.position - b.position;
	Vector2f relativeVelocity = a.velocity - b.velocity;

    float produsScalar = distance.x * relativeVelocity.x + distance.y * relativeVelocity.y;
    // if is negative, the boids are moving towards each other
	return produsScalar < 0;

}

Vector2f separation(Boid& target)
{
    Vector2f steer = { 0.f, 0.f };
    for (int i = 0; i<boidsCnt; i++)
    {
        Boid& boid = boids[i];
        if (&boid == &target) continue;
        if (isAlmostColliding(target, boid))
            steer += target.position - boid.position ;
    }

    return steer;
}

Vector2f alignment(Boid& target)
{
    Vector2f steer = target.velocity;
    float count = 1.f;

    for (int i = 0; i < boidsCnt; i++)
    {
        Boid& boid = boids[i];
        if (&boid == &target) continue;
        Vector2f diff = boid.position - target.position;
        if (diff.x * diff.x + diff.y * diff.y < 25*25)
        {
            steer += boid.velocity;
            count++;
        }
    }

    steer /= count;

    return steer;
}





float PREDATOR_RADIUS = 500.f;
const float predatorWeight = 500.0f;
Vector2f mousePos;

Vector2f predatorAvoidance(Boid& target, Vector2f predatorPos)
{
    Vector2f diff = target.position - predatorPos;
    float distSq = diff.x * diff.x + diff.y * diff.y;

    if (distSq < PREDATOR_RADIUS * PREDATOR_RADIUS)
    {
        float dist = std::sqrt(distSq);
        Vector2f fleeDir = diff / dist * (PREDATOR_RADIUS - dist) / PREDATOR_RADIUS;
        return fleeDir;
    }
    return Vector2f(0.f, 0.f);
}

void Boid::update(float deltaTime)
{
    //SEPARATION
    Vector2f separationSteer = { 0.f, 0.f };

    //ALIGMENT
    Vector2f aligmentSteer = this->velocity;
    float aligmentCount = 1.f;

    //COHESION
    Vector2f desiredPosition = this->position;
    float cohesionCount = 1.f;

    for (int i = 0; i < boidsCnt; i++) //using one for loop for all three behaviours. Uglier but faster
    {
        Boid& boid = boids[i];
        if (&boid == this) continue;

        //SEPARATION
        if (isAlmostColliding(*this, boid))
            separationSteer += this->position - boid.position;

        //ALIGMENT
        Vector2f diff = boid.position - this->position;
        if (diff.x * diff.x + diff.y * diff.y < 25 * 25)
        {
            aligmentSteer += boid.velocity;
            ++aligmentCount;
        }

        //COHESION
        if (diff.x * diff.x + diff.y * diff.y < 50.f * 50.f)
        {
            desiredPosition += boid.position;
            ++cohesionCount;
		}


    }

    desiredPosition /= cohesionCount;

    Vector2f cohesionSteer = desiredPosition - this->position;
    aligmentSteer /= aligmentCount;


	Vector2f steer = separationSteer * separationWeight
                    + aligmentSteer * alignmentWeight
                    + cohesionSteer * cohesionWeight;

	steer += predatorAvoidance(*this, mousePos) * predatorWeight;

	velocity += steer * deltaTime;

    float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);

    if (speed > maxSpeed) 
    {
        velocity = velocity / speed * maxSpeed;
    }
    else if (speed < minSpeed) 
    {
        velocity = velocity / speed * minSpeed;
	}
  


    position += velocity * deltaTime;


    // wrap
    if (wrap)
    {
        if (position.x < 0) position.x += 1920;
        else if (position.x > 1920) position.x -= 1920;

        if (position.y < 0) position.y += 1080;
        else if (position.y > 1080) position.y -= 1080;
    }
    else
    {
        //Bounce
        if (position.x < 0)
        {
            position.x = 0;
            velocity.x = -velocity.x;
        }
        else if (position.x > 1920)
        {
            position.x = 1920;
            velocity.x = -velocity.x;
        }

        if (position.y < 0)
        {
            position.y = 0;
            velocity.y = -velocity.y;
        }
        else if (position.y > 1080)
        {
            position.y = 1080;
            velocity.y = -velocity.y;
        }
    }



}

std::mt19937 rng(std::random_device{}()); // Create a random number generator


Font font("arial.ttf");
Text fps(font);

bool running = 1;


void handleCommands()
{
    using namespace std;
    std::cout << "For a list of commands, type 'help'\n";
    while (running)
    {
        string command;
        getline(cin, command);

        for (char & c : command)
        {
            c = tolower(c);
		}

        if (command == "fps")
        {
            string s = fps.getString();
            cout << s << '\n';
        }
        else if (command == "toggle warp" || command=="tw")
        {
            wrap = !wrap;
			cout << "The boids now " << (wrap ? "warp" : "bounce") << " at the edges of the screen.\n";
        }
        else if (command == "weights view" || command=="wv")
        {
			cout << "Separation weight: " << separationWeight << '\n';
			cout << "Alignment weight: " << alignmentWeight << '\n';
			cout << "Cohesion weight: " << cohesionWeight << '\n';
        }
        else if (command == "weights modify" || command == "wm")
        {
			cout << "Enter the weights in the format: separation alignment cohesion\n";
			cout << "Current weights: " << separationWeight << ", " << alignmentWeight << ", " << cohesionWeight << '\n';
			float newSeparation, newAlignment, newCohesion;
			cin >> newSeparation >> newAlignment >> newCohesion;
			separationWeight = newSeparation;
			alignmentWeight = newAlignment;
            cohesionWeight = newCohesion;
        }
        else if (command == "mouse")
        {
            cout << "Do you want the mouse to be avoided by the boids? (Y/N)\n";
            char choice;
            cin >> choice;
            choice = tolower(choice);
            if (choice == 'y')
            {
                cout << "Mouse avoidance enabled.\n";
                PREDATOR_RADIUS = 500.f;

            }
            else if (choice == 'n')
            {
                cout << "Mouse avoidance disabled.\n";
                PREDATOR_RADIUS = 0.f;
            }
            else
            {
                cout << "Invalid choice. Pls enter Y or N. :p\n";
                PREDATOR_RADIUS = 500.f;
            }
        }
        else if (command == "boids")
        {
			cout << "Current number of boids: " << boidsCnt << '\n';
            cout << "Enter number of boids you wish to exist (0-5000): ";
            cin >> boidsCnt;
            boidsCnt = max(0, boidsCnt);
            boidsCnt = min(5000, boidsCnt);
            return;
        }
        else if (command == "help")
        {
            cout << "Available commands:\n";
            cout << "fps - Show the current FPS\n";
            cout << "toggle warp (tw) - Toggle between wrapping and bouncing at the edges of the screen\n";
            cout << "weights view (wv) - View the current weights for separation, alignment, and cohesion\n";
            cout << "weights modify (wm) - Modify the weights for separation, alignment, and cohesion\n";
            cout << "mouse - Toggle mouse avoidance on or off\n";
            cout << "boids - Change the number of boids in the simulation (0-2000)\n";
		    cout << "help - Show this help message! :D\n\n";
            cout << "Note: For the commands with shorter versions in parentheses,\nyou can use either the full or short versions.\n";

        }
        else
        {
            std::cout << "Unknown command. Type 'help' for a list of commands.\n";
		}
    }
}

int main()
{
    auto window = sf::RenderWindow(sf::VideoMode({1920u, 1080u}), "Boids 2D");
    window.setFramerateLimit(144);

    for (auto& boid : boids)
    {
        boid.position = { rng() % 1920 * 1.f, rng() % 1080 * 1.f };
        boid.velocity = { (rng() % 200 - 100) * 1.f, (rng() % 200 - 100) * 1.f };
    }

    std::thread inputThread(handleCommands);

    while (window.isOpen())
    {
        float dt = deltaClock.restart().asSeconds();
        mousePos = Vector2f(Mouse::getPosition(window));

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                running = 0;
                window.close();
            }
        }

        fps.setString("FPS: " + std::to_string(((int)(1/dt)) * 1));

        window.clear();
        window.draw(fps);

        for (int i = 0; i < boidsCnt; i++)
        {
            Boid& boid = boids[i];
            boid.update(dt);
            boid.draw(window);
        }

        

        window.display();
    }
}
