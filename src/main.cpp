#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
using namespace sf;

const float NEIGHBOUR_RADIUS = 25.F;


struct Boid {
    Vector2f position;
    Vector2f velocity = { 20.f, 20.f };
    float maxSpeed = 500.f;
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
        CircleShape shape(5.f);
        shape.setPosition(position);
        shape.setFillColor(Color::Green);
        window.draw(shape);
    }
};

Clock deltaClock;

Boid boids[500];


bool isColliding(const Boid &a, const Boid& b)
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
    for (auto& boid : boids)
    {
        if (&boid == &target) continue;
        if (isColliding(target, boid))
            steer += target.position - boid.position ;
    }
    return steer;
}

void Boid::update(float deltaTime)
{
	Vector2f steer = separation(*this);

	velocity += steer;

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

    if (position.x < 0 || position.x > 1080) velocity.x *= -1;
    if (position.y < 0 || position.y > 720)  velocity.y *= -1;

}


int main()
{
    auto window = sf::RenderWindow(sf::VideoMode({1080u, 720u}), "Boids 2D");
    window.setFramerateLimit(144);


    for (auto& boid : boids)
    {
        boid.position = { rand() % 1080 * 1.f, rand() % 720 * 1.f };
		boid.velocity = { (rand() % 200 - 100) * 1.f, (rand() % 200 - 100) * 1.f };
    }

    while (window.isOpen())
    {

		float dt = deltaClock.restart().asSeconds();

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        window.clear();

        for (auto &boid : boids)
        {
            boid.update(dt);
            boid.draw(window);
		}
        

        window.display();
    }
}
