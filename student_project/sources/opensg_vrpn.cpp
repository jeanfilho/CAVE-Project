#include <cstdlib>
#include <cstddef>
#include <cmath>
#include <iostream>
#include <ios>
#include <ctime>

#include <OpenSG/OSGGLUT.h>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGGLUTWindow.h>
#include <OpenSG/OSGMultiDisplayWindow.h>
#include <OpenSG/OSGSceneFileHandler.h>

#include <OSGCSM/OSGCAVESceneManager.h>
#include <OSGCSM/OSGCAVEConfig.h>
#include <OSGCSM/appctrl.h>

#include <vrpn_Tracker.h>
#include <vrpn_Button.h>
#include <vrpn_Analog.h>

#include "Scene.h"
#include "TimeManager.h"

OSG_USING_NAMESPACE

OSGCSM::CAVEConfig cfg;
OSGCSM::CAVESceneManager *mgr = nullptr;
vrpn_Tracker_Remote* tracker =  nullptr;
vrpn_Button_Remote* button = nullptr;
vrpn_Analog_Remote* analog = nullptr;

Scene scene = Scene();
TimeManager timeMgr = TimeManager();
float horizontalSpeed = 20;
float verticalSpeed = 20;
float mouseSensitivity = 1;
float amountToMoveX, amountToMoveZ, amountToMoveY;

bool isMouseWarped;
Quaternion rotation;

void cleanup()
{
	delete mgr;
	delete tracker;
	delete button;
	delete analog;
}

void print_tracker();

template<typename T>
T scale_tracker2cm(const T& value)
{
	static const float scale = OSGCSM::convert_length(cfg.getUnits(), 1.f, OSGCSM::CAVEConfig::CAVEUnitCentimeters);
	return value * scale;
}

auto head_orientation = Quaternion(Vec3f(0.f, 1.f, 0.f), 3.141f);
auto head_position = Vec3f(0.f, 170.f, 200.f);	// a 1.7m Person 2m in front of the scene

void VRPN_CALLBACK callback_head_tracker(void* userData, const vrpn_TRACKERCB tracker)
{
	head_orientation = Quaternion(tracker.quat[0], tracker.quat[1], tracker.quat[2], tracker.quat[3]);
	head_position = Vec3f(scale_tracker2cm(Vec3d(tracker.pos)));
}

auto wand_orientation = Quaternion();
auto wand_position = Vec3f();
void VRPN_CALLBACK callback_wand_tracker(void* userData, const vrpn_TRACKERCB tracker)
{
	wand_orientation = Quaternion(tracker.quat[0], tracker.quat[1], tracker.quat[2], tracker.quat[3]);
	wand_position = Vec3f(scale_tracker2cm(Vec3d(tracker.pos)));
}

auto analog_values = Vec3f();
void VRPN_CALLBACK callback_analog(void* userData, const vrpn_ANALOGCB analog)
{
	if (analog.num_channel >= 2)
		analog_values = Vec3f(analog.channel[0], 0, -analog.channel[1]);
}

void VRPN_CALLBACK callback_button(void* userData, const vrpn_BUTTONCB button)
{
	if (button.button == 0 && button.state == 1)
		print_tracker();
}

void InitTracker(OSGCSM::CAVEConfig &cfg)
{
	try
	{
		const char* const vrpn_name = "DTrack@localhost";
		tracker = new vrpn_Tracker_Remote(vrpn_name);
		tracker->shutup = true;
		tracker->register_change_handler(NULL, callback_head_tracker, cfg.getSensorIDHead());
		tracker->register_change_handler(NULL, callback_wand_tracker, cfg.getSensorIDController());
		button = new vrpn_Button_Remote(vrpn_name);
		button->shutup = true;
		button->register_change_handler(nullptr, callback_button);
		analog = new vrpn_Analog_Remote(vrpn_name);
		analog->shutup = true;
		analog->register_change_handler(NULL, callback_analog);
	}
	catch(const std::exception& e) 
	{
		std::cout << "ERROR: " << e.what() << '\n';
		return;
	}
}

void check_tracker()
{
	tracker->mainloop();
	button->mainloop();
	analog->mainloop();

}

void print_tracker()
{
	std::cout << "Head position: " << head_position << " orientation: " << head_orientation << '\n';
	std::cout << "Wand position: " << wand_position << " orientation: " << wand_orientation << '\n';
	std::cout << "Analog: " << analog_values << '\n';
}

void keyboard(unsigned char k, int x, int y)
{
	Real32 ed;
	switch(k)
	{
		case 'q':
		case 27: 
			cleanup();
			exit(EXIT_SUCCESS);
			break;
		case 'e':
			ed = mgr->getEyeSeparation() * .9f;
			std::cout << "Eye distance: " << ed << '\n';
			mgr->setEyeSeparation(ed);
			break;
		case 'E':
			ed = mgr->getEyeSeparation() * 1.1f;
			std::cout << "Eye distance: " << ed << '\n';
			mgr->setEyeSeparation(ed);
			break;
		case 'h':
			cfg.setFollowHead(!cfg.getFollowHead());
			std::cout << "following head: " << std::boolalpha << cfg.getFollowHead() << '\n';
			break;
		case 'i':
			print_tracker();
			break;

		/* Movement */
		case 'w':
			amountToMoveZ = -horizontalSpeed * timeMgr.deltaTime();
			break;
		case 's':
			amountToMoveZ = horizontalSpeed * timeMgr.deltaTime();
			break;
		case 'd':
			amountToMoveX = horizontalSpeed * timeMgr.deltaTime();
			break;
		case 'a':
			amountToMoveX = -horizontalSpeed * timeMgr.deltaTime();
			break;
		case 32:
			amountToMoveY = verticalSpeed * timeMgr.deltaTime();
			break;
		case 'c':
			amountToMoveY = -verticalSpeed * timeMgr.deltaTime();
			break;
		default:
			std::cout << "Key '" << k << "' ignored\n";
	}
}

void keyboardUp(unsigned char k, int x, int y)
{
	switch(k)
	{
		/* Movement stop*/
		case 'w':
		case 'W':
		case 's':
		case 'S':
			amountToMoveZ = 0;
			break;
		case 'd':
		case 'D':
		case 'a':
		case 'A':
			amountToMoveX = 0;
			break;
		case 32:
		case 'c':
		case 'C':
			amountToMoveY = 0;
			break;
		default:
			std::cout << "Key '" << k << "' ignored\n";
	}
}

void mouse(int button, int state, int x, int y)
{
	switch(button)
	{
		case GLUT_LEFT_BUTTON:
			if(state == GLUT_DOWN)
			{
				Quaternion direction = Quaternion(0,0,-1,0);
				direction = (rotation * direction) * rotation.conj();
				Vec3f normDirection = Vec3f(direction.x(), direction.y(), direction.z());
				normDirection.normalize();
				scene.throwCoconut(mgr->getTranslation(), normDirection);
			}
			break;
		default:
			break;
	}

}

void motionPassive(int x, int y)
{
	/* Mouse Rotation */
	if(!isMouseWarped)
	{
		int middleX = glutGet(GLUT_WINDOW_WIDTH)/2;
		int middleY = glutGet(GLUT_WINDOW_HEIGHT)/2;
	
		int deltaX = x - middleX;
		int deltaY = y - middleY;
		float rotateY =-deltaX * timeMgr.deltaTime() * mouseSensitivity;

		rotation *= Quaternion(Vec3f(0,1,0), rotateY);
		rotation.normalize();	
		mgr->addYRotate(rotateY);
		
		isMouseWarped = true;
		glutWarpPointer(middleX, middleY);		
	}
	else
		isMouseWarped = false;
	
}

void motion(int x, int y)
{
	motionPassive(x, y);
}

void setupGLUT(int *argc, char *argv[])
{
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_RGB  |GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("OpenSG CSMDemo with VRPN API");
	glutDisplayFunc([]()
	{
		// black navigation window
		glClear(GL_COLOR_BUFFER_BIT);
		glutSwapBuffers();
	});
	glutReshapeFunc([](int w, int h)
	{
		mgr->resize(w, h);
		glutPostRedisplay();
	});
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motionPassive);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutMouseFunc(mouse);
	glutIdleFunc([]()
	{
		timeMgr.update();

		check_tracker();
		const auto speed = 1.f;
		mgr->setUserTransform(head_position, head_orientation);
		mgr->setTranslation(mgr->getTranslation() + speed * analog_values);
		
		/* Keyboard Translation */
		Quaternion translation = Quaternion(amountToMoveX, amountToMoveY, amountToMoveZ, 0);
		translation = (rotation * translation) * rotation.conj();
		mgr->setTranslation(mgr->getTranslation() + Vec3f(translation.x() * horizontalSpeed, translation.y() * verticalSpeed, translation.z() * horizontalSpeed));

		Vec3f pos = mgr->getTranslation();
		Vec3f up = Vec3f(0,1,0);
		Vec3f right = Vec3f(1,0,0);
		Vec3f fwd = Vec3f(0,0,1);
		mgr->getYRotate();
		
		/* Update scene */
		Matrix4f viewMatrix = Matrix4f();
		viewMatrix.setTranslate(-mgr->getTranslation());
		viewMatrix.setRotate(Quaternion(Vec3f(0,1,0), mgr->getYRotate()));
		viewMatrix.invert();
		scene.update(timeMgr.deltaTime(), viewMatrix);

		commitChanges();
		mgr->redraw();
		// the changelist should be cleared - else things could be copied multiple times
		OSG::Thread::getCurrentChangeList()->clear();
	});
}

void setupBackground(SkyBackgroundRecPtr bg)
{
	/* Front */
	ImageRecPtr img = Image::create();
	img->read("textures/Front.2048x2048.png");
	TextureObjChunkRecPtr tex = TextureObjChunk::create();
	tex->setImage(img);
	bg->setFrontTexture(tex);

	/* Back */
	img = Image::create();
	img->read("textures/Back.2048x2048.png");
	tex = TextureObjChunk::create();
	tex->setImage(img);
	bg->setBackTexture(tex);

	/* Left */
	img = Image::create();
	img->read("textures/Right.2048x2048.png");
	tex = TextureObjChunk::create();
	tex->setImage(img);
	bg->setLeftTexture(tex);

	/* Right */
	img = Image::create();
	img->read("textures/Left.2048x2048.png");
	tex = TextureObjChunk::create();
	tex->setImage(img);
	bg->setRightTexture(tex);

	/* Up */
	img = Image::create();
	img->read("textures/Up.2048x2048.png");
	tex = TextureObjChunk::create();
	tex->setImage(img);
	bg->setTopTexture(tex);

	/* Back */
	img = Image::create();
	img->read("textures/Down.2048x2048.png");
	tex = TextureObjChunk::create();
	tex->setImage(img);
	bg->setBottomTexture(tex);
}

int main(int argc, char **argv)
{
#if WIN32 
	OSG::preloadSharedObject("OSGFileIO");
	OSG::preloadSharedObject("OSGImageFileIO");
#endif
	try
	{
		bool cfgIsSet = false;
		NodeRefPtr gameScene = nullptr;

		// ChangeList needs to be set for OpenSG 1.4
		ChangeList::setReadWriteDefault();
		osgInit(argc,argv);

		// evaluate intial params
		for(int a=1 ; a<argc ; ++a)
		{
			if( argv[a][0] == '-' )
			{
				if ( strcmp(argv[a],"-f") == 0 ) 
				{
					char* cfgFile = argv[a][2] ? &argv[a][2] : &argv[++a][0];
					if (!cfg.loadFile(cfgFile)) 
					{
						std::cout << "ERROR: could not load config file '" << cfgFile << "'\n";
						return EXIT_FAILURE;
					}
					cfgIsSet = true;
				}
			} else {
				std::cout << "Loading scene file '" << argv[a] << "'\n";
				gameScene = SceneFileHandler::the()->read(argv[a], NULL);
			}
		}

		// load the CAVE setup config file if it was not loaded already:
		if (!cfgIsSet) 
		{
			const char* const default_config_filename = "config/mono.csm";
			if (!cfg.loadFile(default_config_filename)) 
			{
				std::cout << "ERROR: could not load default config file '" << default_config_filename << "'\n";
				return EXIT_FAILURE;
			}
		}

		cfg.printConfig();

		// start servers for video rendering
		if ( startServers(cfg) < 0 ) 
		{
			std::cout << "ERROR: Failed to start servers\n";
			return EXIT_FAILURE;
		}

		setupGLUT(&argc, argv);

		InitTracker(cfg);

		MultiDisplayWindowRefPtr mwin = createAppWindow(cfg, cfg.getBroadcastaddress());

		if (!gameScene) 
		{
			//Initialize own game scene
			scene.initialize();
			gameScene = scene.getBase();
		}
		commitChanges();

		/* Background */
		SkyBackgroundRecPtr bg = SkyBackground::create();
		setupBackground(bg);
		
		mgr = new OSGCSM::CAVESceneManager(&cfg);
		mgr->setWindow(mwin);
		mgr->setRoot(gameScene);
		mgr->showAll();
		mgr->getWindow()->init();
		mgr->turnWandOff();
		mgr->setHeadlight(false);
		mgr->addToTrolley(makeSphere(1,10), 0);
		mgr->setBackground(0,bg); 
	}
	catch(const std::exception& e)
	{
		std::cout << "ERROR: " << e.what() << '\n';
		return EXIT_FAILURE;
	}

	glutMainLoop();
}
