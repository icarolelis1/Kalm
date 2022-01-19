#include "Kalm.h"
#pragma warning( push )
#pragma warning( disable : C26812)
// Some code
Kalm::Kalm(const char* _name):name(_name)
{
}

void Kalm::initiateResouces()
{	
	WindowProperties properties;
	properties._HEIGHT = 	1080;
	properties._WIDTH = 1920;
	properties._title = "TEST";

	//Initiate Window
	window.initiateWindow(properties);
	//Initiate Graphics Resources
	render.initiateResources(window.getWindowHandler(),properties._WIDTH,properties._HEIGHT);



}

void Kalm::loop()
{

}

Kalm::~Kalm()
{
	window.destroyWindow();
	
}

int main() {
	
	//App execution
	Kalm kalm("TESTE");
	kalm.initiateResouces();
	kalm.loop();
	


}
#pragma warning( pop )
