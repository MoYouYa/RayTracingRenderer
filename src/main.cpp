#include "Sphere.h"
#include "Render.h"
#include "Trangle.h"

void main() {

	Material* whiteLightMaterial = new Material(MaterialType::DIFFUSE, 8.0f * Vector3f(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) + 15.6f * Vector3f(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) + 18.4f * Vector3f(0.737f + 0.642f, 0.737f + 0.159f, 0.737f));

	Material* whiteDiffuseMaterial = new Material(MaterialType::DIFFUSE, Vector3f(0), Vector3f(0.6, 0.6, 0.6));
	Material* redDiffuseMaterial = new Material(MaterialType::DIFFUSE, Vector3f(0),Vector3f(0.6,0.01,0.01));
	Material* greenDiffuseMaterial = new Material(MaterialType::DIFFUSE, Vector3f(0), Vector3f(0.01, 0.6, 0.01));
	Material* blueDiffuseMaterial = new Material(MaterialType::DIFFUSE, Vector3f(0), Vector3f(0.01, 0.01, 0.6));
	Material* yellowDiffuseMaterial = new Material(MaterialType::DIFFUSE, Vector3f(0), Vector3f(0.6, 0.6, 0.01));
	Material* pinkDiffuseMaterial = new Material(MaterialType::DIFFUSE, Vector3f(0), Vector3f(0.01, 0.6, 0.6));
	Material* cyanDiffuseMaterial = new Material(MaterialType::DIFFUSE, Vector3f(0), Vector3f(0.6, 0.01, 0.6));

	Material* specularMaterial = new Material(MaterialType::SPECULAR, Vector3f(0),Vector3f(1));

	Material* microfactMaterial0 = new Material(MaterialType::MICROFACET, Vector3f(0), Vector3f(0.6f, 0.01f, 0.01f), Vector3f(0.6f, 0.01f, 0.01f), 0, 0.5f);


	std::vector<Vector3f> cube ={ 
		Vector3f(0,0,-512),Vector3f(512,0,-512),Vector3f(0,512,-512),Vector3f(512,512,-512),
		Vector3f(0,0,0),Vector3f(512,0,0),Vector3f(0,512,0),Vector3f(512,512,0) };
	std::vector<unsigned int> redBackIndices = {0,1,2,3,2,1};
	TrangleMesh* Back = new TrangleMesh(redDiffuseMaterial, &cube, &redBackIndices);

	std::vector<unsigned int> blueTopIndices = { 0,4,1,5,1,4};
	TrangleMesh* Top = new TrangleMesh(whiteDiffuseMaterial, &cube, &blueTopIndices);

	std::vector<unsigned int> greenBottomIndices = { 2,3,6,7,6,3};
	TrangleMesh* Bottom = new TrangleMesh(yellowDiffuseMaterial, &cube, &greenBottomIndices);

	std::vector<unsigned int> whiteLeftIndices = { 0,2,4,6,4,2};
	TrangleMesh* Left = new TrangleMesh(pinkDiffuseMaterial, &cube, &whiteLeftIndices);

	std::vector<unsigned int> whiteRightIndices = { 1,5,3,7,3,5};
	TrangleMesh* Right = new TrangleMesh(cyanDiffuseMaterial, &cube, &whiteRightIndices);

	Sphere* sphereLight = new Sphere( whiteLightMaterial, Vector3f(256,256 , -256), 20);

	std::vector<Vector3f> plate = { Vector3f(256-32,20,-256-32),Vector3f(256 + 32,20,-256 - 32),Vector3f(256-32,20,-256+32),Vector3f(256 + 32,20,-256+32) };
	std::vector<unsigned int> plateLightIndices = {0,1,2,3,2,1 };
	TrangleMesh* plateLight = new TrangleMesh(whiteLightMaterial, &plate, &plateLightIndices);

	std::vector<Vector3f> plateBack = { Vector3f(256 - 32,21,-256 - 32),Vector3f(256 + 32,21,-256 - 32),Vector3f(256 - 32,21,-256 + 32),Vector3f(256 + 32,21,-256 + 32) };
	std::vector<unsigned int> plateLightBackIndices = { 0,2,1,3,1,2 };
	TrangleMesh* plateLightBack= new TrangleMesh(whiteDiffuseMaterial, &plateBack, &plateLightBackIndices);


	Sphere* redSphere= new Sphere(redDiffuseMaterial, Vector3f(256, 512-64, -256), 32);
	Sphere* blueSphere = new Sphere(blueDiffuseMaterial, Vector3f(64, 256, -256), 32);
	Sphere* greenSphere = new Sphere(greenDiffuseMaterial, Vector3f(512-64, 256, -256), 32);
	Sphere* microfactSphere = new Sphere(microfactMaterial0, Vector3f(256, 64, -256), 32);
	Sphere* specularSphere1 = new Sphere(specularMaterial, Vector3f(64, 64, -512 + 64), 64);
	Sphere* specularSphere2 = new Sphere(specularMaterial, Vector3f(64, 512 - 64, -512 + 64), 64);
	Sphere* specularSphere3 = new Sphere(specularMaterial, Vector3f(512 - 64, 64, -512 + 64), 64);
	Sphere* specularSphere4 = new Sphere(specularMaterial, Vector3f(512 - 64, 512 - 64, -512 + 64), 64);

	Camera* camera = new Camera(Vector3f(256, 256, 512), Vector3f(0, 0, -1), Vector3f(0, 1, 0), 45);

	Scene scene(camera);

	scene.addObject((Object*)sphereLight);
	//scene.addObject((Object*)plateLight);
	//scene.addObject((Object*)plateLightBack);

	scene.addObject((Object*)Back);
	scene.addObject((Object*)Top);
	scene.addObject((Object*)Bottom);
	scene.addObject((Object*)Left);
	scene.addObject((Object*)Right);

	scene.addObject((Object*)redSphere);
	scene.addObject((Object*)blueSphere);
	scene.addObject((Object*)greenSphere);

	scene.addObject((Object*)microfactSphere);

	scene.addObject((Object*)specularSphere1);
	scene.addObject((Object*)specularSphere2);
	scene.addObject((Object*)specularSphere3);
	scene.addObject((Object*)specularSphere4);
	scene.rebuildBVH();

	Render renderer;
	//renderer.render(scene, 1024,1024, RenderType::BIDIRECTIONAL_PATH_TRACING,32,"images/plateLight_BDPT");
	//renderer.render(scene, 1024, 1024, RenderType::PATH_TRACING, 1, "images/plateLight_FacetoWall_test");
	//renderer.render(scene, 1024, 1024, RenderType::BIDIRECTIONAL_PATH_TRACING, 32, "images/plateLight_FacetoWall_BDPT_test");
	renderer.render(scene, 512, 512, RenderType::BIDIRECTIONAL_PATH_TRACING, 16, "images/plateLight_FacetoWall_BDPT_s-1_t0_smaxn_tmaxn_diffuse_and_specular_microfact_new");
	//renderer.render(scene, 512, 512, RenderType::PATH_TRACING, 64, "images/plateLight_FacetoWall_diffuse_and_specular_microfact");
	//renderer.render(scene, 512, 512, RenderType::BIDIRECTIONAL_PATH_TRACING, 4, "images/sphereLight_BDPT_MIS");
	//renderer.render(scene, 512, 512, RenderType::PATH_TRACING, 4, "images/sphereLight");




	//cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
	//image.convertTo(image, CV_8UC3, 1.0f);
	//cv::imshow("testImage", image);
	//cv::imwrite("test.png", image);
	//cv::waitKey(0);
	return;
}