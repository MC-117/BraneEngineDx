#include "PostProcessCameraRender.h"
#include "Console.h"

PostProcessCameraRender::PostProcessCameraRender(RenderTarget & renderTarget, Material & material) : CameraRender(renderTarget, material)
{
	/*ssaoTarget.addTexture("ssaoMap", ssaoTex);
	ssaoBlurXTarget.addTexture("ssaoMap", ssaoBlurXTex);
	ssaoBlurYTarget.addTexture("ssaoMap", ssaoBlurYTex);
	screenTarget.addTexture("screenMap", screenTex);
	screenBlurTarget.addTexture("screenMap", screenBlurTex);
	screenBlurXTarget.addTexture("screenMap", screenBlurXTex);
	screenBlurYTarget.addTexture("screenMap", screenBlurYTex);
	bloomTarget.addTexture("screenMap", screenTex);
	volumnicLightTarget.addTexture("screenMap", volumnicLightTex);
	volumnicLightXTarget.addTexture("volumnicFogMap", volumnicLightXTex);
	for (int i = 0; i < 6; i++) {
		Texture2D* t = new Texture2D(size.x / pow(2, i), size.y / pow(2, i), 4, true, { GL_CLAMP, GL_CLAMP, GL_LINEAR, GL_LINEAR });
		RenderTarget* rt = new RenderTarget(size.x / pow(2, i), size.y / pow(2, i), 4);
		Texture2D* tx = new Texture2D(size.x / pow(2, i), size.y / pow(2, i), 4, true, { GL_CLAMP, GL_CLAMP, GL_LINEAR, GL_LINEAR });
		RenderTarget* xrt = new RenderTarget(size.x / pow(2, i), size.y / pow(2, i), 4);
		Texture2D* ty = new Texture2D(size.x / pow(2, i), size.y / pow(2, i), 4, true, { GL_CLAMP, GL_CLAMP, GL_LINEAR, GL_LINEAR });
		RenderTarget* yrt = new RenderTarget(size.x / pow(2, i), size.y / pow(2, i), 4);
		rt->addTexture("screenMap", *t);
		xrt->addTexture("screenMap", *tx);
		yrt->addTexture("screenMap", *ty);
		char str[15];
		sprintf(str, "bloomMap_%d", i);
		bloomTarget.addTexture(str, *ty);
		bloomList.push_back({ t, rt, tx, xrt, ty, yrt });
	}
	screenTarget.addTexture("bloomMap_0", *bloomList[0].bloomBlurTex);*/

	graph.addDefaultPasses();
}

PostProcessCameraRender::~PostProcessCameraRender()
{
	/*for (int i = 0; i < bloomList.size(); i++) {
		delete bloomList[i].bloomBlurTarget;
		delete bloomList[i].bloomBlurTex;
		delete bloomList[i].tempXTarget;
		delete bloomList[i].tempXTex;
		delete bloomList[i].tempYTarget;
		delete bloomList[i].tempYTex;
	}*/
}

Texture2D * PostProcessCameraRender::getSceneBlurTex()
{
	//return &screenBlurYTex;
	return NULL;
}

void PostProcessCameraRender::setSize(Unit2Di size)
{
	if (size != this->size) {
		if (!renderTarget.isDefault()) {
			renderTarget.resize(size.x, size.y);
			////depthTarget.resize(size.x, size.y);
			//ssaoTarget.resize(size.x * SSAORES, size.y * SSAORES);
			//ssaoBlurXTarget.resize(size.x * SSAORES, size.y * SSAORES);
			//ssaoBlurYTarget.resize(size.x * SSAORES, size.y * SSAORES);
			//screenTarget.resize(size.x, size.y);
			//screenBlurTarget.resize(size.x, size.y);
			//screenBlurXTarget.resize(size.x, size.y);
			//screenBlurYTarget.resize(size.x, size.y);
			//volumnicLightTarget.resize(size.x, size.y);
			//volumnicLightXTarget.resize(size.x, size.y);
			//for (int i = 0; i < bloomList.size(); i++) {
			//	if (i != 0) {
			//		bloomList[i].bloomBlurTarget->resize(size.x / pow(2, i), size.y / pow(2, i));
			//	}
			//	bloomList[i].tempXTarget->resize(size.x / pow(2, i), size.y / pow(2, i));
			//	bloomList[i].tempYTarget->resize(size.x / pow(2, i), size.y / pow(2, i));
			//}
		}

		graph.resize(size);
		this->size = size;
	}
}

void PostProcessCameraRender::setVolumnicLight(DirectLight & light)
{
	volumnicLight = &light;
}

void PostProcessCameraRender::render(RenderInfo & info)
{
	DrawElementsIndirectCommand cmd = { 4, 1, 0, 0 };
	if (isValid() /*&& !material.isNull()*/) {
		renderTarget.SetMultisampleFrame();
		graph.resource.depthTexture = renderTarget.getInternalDepthTexture();
		graph.resource.screenTexture = renderTarget.getTexture(0);
		graph.resource.screenRenderTarget = &renderTarget;
		graph.render(info);

		//ShaderProgram* shaderProgram = material.getShader()->getProgram(Shader_Postprocess);
		//if (shaderProgram == NULL || shaderProgram->bind() == 0)
		//	return;

		//if (volumnicLight != NULL) {
		//	volumnicLight->getShadowRenderTarget()->setTexture(material);
		//	Vector3f &sunDir = volumnicLight->shadowCamera.forward;
		//	material.setColor("sunDir", Color(sunDir.x(), sunDir.y(), sunDir.z(), 1.0f));
		//	volumnicLightTarget.bindFrame();
		//	glClear(GL_COLOR_BUFFER_BIT);
		//	material.setScalar("width", size.x);
		//	material.setScalar("height", size.y);
		//	material.setPass(0);
		//	glViewport(0, 0, size.x, size.y);
		//	material.processShader(projectionViewMat, transformMat, volumnicLight->getLightSpaceMatrix(), cameraLoc, volumnicLight->forward,
		//		[](void *obj, int pid) {
		//		int fLoc = Shader::getAttributeIndex(pid, "fovy");
		//		int zN = Shader::getAttributeIndex(pid, "zNear");
		//		int zF = Shader::getAttributeIndex(pid, "zFar");
		//		int proLoc = Shader::getAttributeIndex(pid, "promat");
		//		int vLoc = Shader::getAttributeIndex(pid, "vmat");
		//		if (fLoc != -1)
		//			glProgramUniform1f(pid, fLoc, ((RenderInfo*)obj)->fovy);
		//		if (zN != -1)
		//			glProgramUniform1f(pid, zN, ((RenderInfo*)obj)->camera->zNear);
		//		if (zF != -1)
		//			glProgramUniform1f(pid, zF, ((RenderInfo*)obj)->camera->zFar);
		//		if (proLoc != -1) {
		//			Matrix4f promat = ((RenderInfo*)obj)->camera->getProjectionMatrix();
		//			glProgramUniformMatrix4fv(pid, proLoc, 1, 0, promat.data());
		//		}
		//		if (vLoc != -1) {
		//			Matrix4f vmat = ((RenderInfo*)obj)->camera->getViewMatrix();
		//			glProgramUniformMatrix4fv(pid, vLoc, 1, 0, vmat.data());
		//		}

		//	}, &info);
		//	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//	glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);

		//	volumnicLightXTarget.bindFrame();
		//	volumnicLightTarget.setTexture(material);
		//	glClear(GL_COLOR_BUFFER_BIT);
		//	material.setPass(1);
		//	glViewport(0, 0, size.x, size.y);
		//	material.processShader(info.camera->getProjectionMatrix(), transformMat, info.lightSpaceMat, cameraLoc, cameraDir);
		//	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//	glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);
		//}

		///*glBindFramebuffer(GL_READ_FRAMEBUFFER, renderTarget.getFBO());
		//int tex = depthTex.bind();
		//glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 0, 0, depthTex.getWidth(), depthTex.getHeight(), 0);
		//glBindTexture(GL_TEXTURE_2D, 0);*/

		////material.setScalar("ssaoIntensity", 0);

		///*depthTarget.bindFrame();
		//depthTarget.blitFrom(renderTarget, GL_COLOR_BUFFER_BIT);*/

		//material.setScalar("width", size.x * SSAORES);
		//material.setScalar("height", size.y * SSAORES);

		//ssaoTarget.bindFrame();
		////renderTarget.setTexture(material);
		////material.setTexture("normalMap", *renderTarget.getTexture("normalMap"));
		//if (renderTarget.getInternalDepthTexture() == NULL)
		//	material.setScalar("ssaoIntensity", 0);
		//else
		//	material.setTexture("depthMap", *renderTarget.getInternalDepthTexture());
		////depthTarget.setTexture(material);
		//glClear(GL_COLOR_BUFFER_BIT);
		//material.setPass(10);
		//glViewport(0, 0, size.x * SSAORES, size.y * SSAORES);
		//material.processShader(projectionViewMat, transformMat, info.lightSpaceMat, cameraLoc, cameraDir);
		////glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);

		//ssaoBlurXTarget.bindFrame();
		//ssaoTarget.setTexture(material);
		//glClear(GL_COLOR_BUFFER_BIT);
		//material.setPass(11);
		//glViewport(0, 0, size.x * SSAORES, size.y * SSAORES);
		//material.processShader(projectionViewMat, transformMat, info.lightSpaceMat, cameraLoc, cameraDir);
		////glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);

		//ssaoBlurYTarget.bindFrame();
		//ssaoBlurXTarget.setTexture(material);
		//glClear(GL_COLOR_BUFFER_BIT);
		//material.setPass(12);
		//glViewport(0, 0, size.x * SSAORES, size.y * SSAORES);
		//material.processShader(projectionViewMat, transformMat, info.lightSpaceMat, cameraLoc, cameraDir);
		////glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);

		//material.setScalar("width", size.x);
		//material.setScalar("height", size.y);

		//screenTarget.bindFrame();
		//renderTarget.setTexture(material);
		//volumnicLightXTarget.setTexture(material);
		//ssaoBlurYTarget.setTexture(material);
		//glClear(GL_COLOR_BUFFER_BIT);
		//material.setPass(2);
		//glViewport(0, 0, size.x, size.y);
		//material.processShader(projectionViewMat, transformMat, info.lightSpaceMat, cameraLoc, cameraDir);
		////glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);

		//for (int i = 1; i < bloomList.size(); i++) {
		//	bloomList[0].bloomBlurTarget->setTexture(material);
		//	bloomList[i].bloomBlurTarget->bindFrame();
		//	glClear(GL_COLOR_BUFFER_BIT);
		//	material.setPass(3);
		//	glViewport(0, 0, size.x / pow(2, i), size.y / pow(2, i));
		//	material.processShader(projectionViewMat, transformMat, info.lightSpaceMat, cameraLoc, cameraDir);
		//	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//	glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);
		//}

		//for (int i = 0; i < bloomList.size(); i++) {
		//	material.setScalar("width", size.x / pow(2, i));
		//	material.setScalar("height", size.y / pow(2, i));

		//	bloomList[i].bloomBlurTarget->setTexture(material);
		//	bloomList[i].tempXTarget->bindFrame();
		//	glClear(GL_COLOR_BUFFER_BIT);
		//	material.setPass(4);
		//	glViewport(0, 0, size.x / pow(2, i), size.y / pow(2, i));
		//	material.processShader(projectionViewMat, transformMat, info.lightSpaceMat, cameraLoc, cameraDir);
		//	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//	glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);

		//	bloomList[i].tempXTarget->setTexture(material);
		//	bloomList[i].tempYTarget->bindFrame();
		//	glClear(GL_COLOR_BUFFER_BIT);
		//	material.setPass(5);
		//	glViewport(0, 0, size.x / pow(2, i), size.y / pow(2, i));
		//	material.processShader(projectionViewMat, transformMat, info.lightSpaceMat, cameraLoc, cameraDir);
		//	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//	glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);
		//}

		//material.setScalar("width", size.x);
		//material.setScalar("height", size.y);

		//bloomTarget.setTexture(material);
		//screenBlurTarget.bindFrame();
		//material.setPass(6);
		//glClear(GL_COLOR_BUFFER_BIT);
		//glViewport(0, 0, size.x, size.y);
		//material.processShader(projectionViewMat, transformMat, info.lightSpaceMat, cameraLoc, cameraDir);
		////glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);

		//screenBlurTarget.setTexture(material);
		//screenBlurXTarget.bindFrame();
		//material.setPass(7);
		//glClear(GL_COLOR_BUFFER_BIT);
		//glViewport(0, 0, size.x, size.y);
		//material.processShader(projectionViewMat, transformMat, info.lightSpaceMat, cameraLoc, cameraDir);
		////glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);

		//screenBlurXTarget.setTexture(material);
		//screenBlurYTarget.bindFrame();
		//material.setPass(8);
		//glClear(GL_COLOR_BUFFER_BIT);
		//glViewport(0, 0, size.x, size.y);
		//material.processShader(projectionViewMat, transformMat, info.lightSpaceMat, cameraLoc, cameraDir);
		////glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);

		//screenBlurTarget.setTexture(material);
		//material.setTexture("blurMap", screenBlurYTex);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//material.setPass(9);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glViewport(0, 0, size.x, size.y);
		//float z[2] = { info.camera->zNear, info.camera->zFar };
		//material.processShader(projectionViewMat, transformMat, info.lightSpaceMat, cameraLoc, cameraDir,
		//	[](void* p, int pid) {
		//	int zN = Shader::getAttributeIndex(pid, "zNear");
		//	int zF = Shader::getAttributeIndex(pid, "zFar");
		//	if (zN != -1)
		//		glProgramUniform1f(pid, zN, ((float*)p)[0]);
		//	if (zF != -1)
		//		glProgramUniform1f(pid, zF, ((float*)p)[1]);
		//}, z);
		////glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);
	}
}
