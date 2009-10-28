#include "Agent.h"
#include "App.h"

class MainApp : public App
{
	void CreateTestCaseRandom(World& world)
	{

		world.Add(*(new Agent(&world, Vector2D::kZero, Vector2D(4.0f, -10.0f), 1.0f, Color::kWhite)));
		world.Add(*(new Agent(&world, Vector2D(3.0f, 6.0f), Vector2D(16.5f, 1.5f), 1.1f, Color::kWhite)));
		world.Add(*(new Agent(&world, Vector2D(-3.0f, 6.0f), Vector2D(-2.5f, -10.5f), 1.2f, Color::kWhite)));
		world.Add(*(new Agent(&world, Vector2D(-6.0f, -3.0f), Vector2D(3.5f, 6.5f), 1.3f, Color::kWhite)));
		world.Add(*(new Agent(&world, Vector2D(-3.0f, -6.0f), Vector2D(2.5f, 3.5f), 1.4f, Color::kWhite)));
		world.Add(*(new Agent(&world, Vector2D(6.0f, 6.0f), Vector2D(12.5f, -5.5f), 1.5f, Color::kWhite)));
	}

	void CreateTestCase1(World& world)
	{
		world.Add(*(new Agent(&world, Vector2D(-6.0f, 0.0f), Vector2D(1.0f, 0.0f), 1.0f, Color::kWhite)));
		world.Add(*(new Agent(&world, Vector2D::kZero, Vector2D::kZero, 1.4f, Color::kWhite)));
	}

	void CreateTestCase2(World& world)
	{
		world.Add(*(new Agent(&world, Vector2D(-6.0f, 0.0f), Vector2D(1.5f, 0.0f), 1.0f, Color::kBlue)));
		world.Add(*(new Agent(&world, Vector2D(0.0f, -6.0f), Vector2D(0.0f, 1.0f), 1.0f, Color::kWhite)));
	}

	void CreateTestCrossing4(World& world, float speed = 3.0f)
	{
		Agent* pAgent1 = new Agent(&world, Vector2D(-10.0f, -15.0f), Vector2D::kZero, 1.0f, Color::kWhite);
		Agent* pAgent2 = new Agent(&world, Vector2D(-5.0f, -15.0f), Vector2D::kZero, 1.0f, Color::kWhite);
		Agent* pAgent3 = new Agent(&world, Vector2D(0.0f, -15.0f), Vector2D::kZero, 1.0f, Color::kWhite);
		Agent* pAgent4 = new Agent(&world, Vector2D(5.0f, -15.0f), Vector2D::kZero, 1.0f, Color::kWhite);

		pAgent1->SetGoal(Vector2D(pAgent4->GetPos().x, 15.0f), speed);
		pAgent2->SetGoal(Vector2D(pAgent3->GetPos().x, 15.0f), speed);
		pAgent3->SetGoal(Vector2D(pAgent2->GetPos().x, 15.0f), speed);
		pAgent4->SetGoal(Vector2D(pAgent1->GetPos().x, 15.0f), speed);

		world.Add(*pAgent1);
		world.Add(*pAgent2);
		world.Add(*pAgent3);
		world.Add(*pAgent4);
	}

	virtual void OnStart(World& world)
	{
		CreateTestCrossing4(world);
	}
};




int main(int argc, char *argv[])
{
	World world;
	MainApp app;

	world.MainLoop(app);

	return 0;
}
