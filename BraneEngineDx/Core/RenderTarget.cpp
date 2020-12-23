#include "RenderTarget.h"

RenderTarget RenderTarget::defaultRenderTarget;
unsigned int RenderTarget::currentFbo = 0;

RenderTarget::RenderTarget()
{
}

RenderTarget::RenderTarget(int width, int height, int channel, bool withDepthStencil, int multisampleLevel)
{
	this->width = width;
	this->height = height;
	this->channel = channel;
	this->withDepthStencil = withDepthStencil;
	this->multisampleLevel = multisampleLevel;
}

RenderTarget::RenderTarget(int width, int height, Texture & depthTexture)
{
	this->width = width;
	this->height = height;
	this->channel = 1;
	this->depthTexure = &depthTexture;
	depthOnly = true;
}

RenderTarget::~RenderTarget()
{
	if (multisampleIndex != 0)
		glDeleteTextures(1, &multisampleIndex);
}

bool RenderTarget::isValid()
{
	return fbo != 0;
}

bool RenderTarget::isDefault()
{
	return width == 0 && height == 0 && channel == 0 && fbo == 0;
}

bool RenderTarget::isDepthOnly()
{
	return depthOnly;
}

unsigned int RenderTarget::bindFrame()
{
	if (isDefault()) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return 0;
	}
	if (!inited) {
		if (fbo == 0) {
			glGenFramebuffers(1, &fbo);
		}
		if (multisampleLevel != 0) {
			if (multisampleFbo == 0)
				glGenFramebuffers(1, &multisampleFbo);
		}
		if (withDepthStencil && depthIndex == 0) {
			glGenTextures(1, &depthIndex);
			if (multisampleLevel != 0) {
				glGenFramebuffers(1, &multisampleDepthIndex);
			}
		}
		resize(width, height);
		inited = true;
	}
	if (multisampleLevel == 0) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		currentFbo = fbo;
		return fbo;
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, multisampleFbo);
		currentFbo = multisampleFbo;
		return multisampleFbo;
	}
	//if (fbo == 0) {
	//	glGenFramebuffers(1, &fbo);
	//	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	//	if (depthOnly) {
	//		unsigned int index = depthTexure->resize(width, height);
	//		if (index == 0)
	//			throw runtime_error("Bind depth texture failed");
	//		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, index, 0);
	//	}
	//	else {
	//		if (textureList.empty()) {
	//			glDrawBuffer(GL_NONE);
	//			glReadBuffer(GL_NONE);
	//		}
	//		else {
	//			int i = 0;
	//			unsigned int* mode = new unsigned int[textures.size()];
	//			for (auto b = textureList.begin(), e = textureList.end(); b != e && i < GL_MAX_COLOR_ATTACHMENTS; b++, i++) {
	//				unsigned int index = b->second->resize(width, height);
	//				if (index == 0)
	//					throw runtime_error("Bind texture failed");
	//				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, index, 0);
	//				mode[i] = GL_COLOR_ATTACHMENT0 + i;
	//			}
	//			glDrawBuffers(textures.size(), mode);
	//			delete[] mode;
	//		}
	//	}
	//	glBindTexture(GL_TEXTURE_2D, 0);
	//}
	//if (multisampleLevel == 0) {
	//	if (fbo != 0) {
	//		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	//		currentFbo = fbo;
	//	}
	//}
	//else {
	//	if (multisampleFbo != 0) {
	//		glBindFramebuffer(GL_FRAMEBUFFER, multisampleFbo);
	//		currentFbo = multisampleFbo;
	//		return multisampleFbo;
	//	}
	//	else {
	//		glGenFramebuffers(1, &multisampleFbo);
	//		glBindFramebuffer(GL_FRAMEBUFFER, multisampleFbo);
	//		currentFbo = multisampleFbo;
	//	}
	//	if (multisampleIndex == 0) {
	//		glGenTextures(1, &multisampleIndex);
	//		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleIndex);
	//		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisampleLevel, GL_RGB, width, height, GL_TRUE);
	//		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	//		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, multisampleIndex, 0);
	//	}
	//}
	//if (withDepthStencil && depthIndex == 0) {//rbo == 0) {
	//	//glGenRenderbuffers(1, &rbo);
	//	//glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	//	//glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisampleLevel, GL_DEPTH24_STENCIL8, width, height);
	//	////glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisampleLevel, GL_DEPTH_COMPONENT24, width, height);
	//	//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	//	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	//	////glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	//	glGenTextures(1, &depthIndex);
	//	glBindTexture(GL_TEXTURE_2D, depthIndex);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	//	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	//	glBindTexture(GL_TEXTURE_2D, 0);
	//	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthIndex, 0);
	//	internalDepthTexure.assign(width, height, 1, { GL_CLAMP, GL_CLAMP,
	//		GL_NEAREST, GL_NEAREST, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24 }, depthIndex);
	//	if (multisampleLevel != 0) {
	//		glGenTextures(1, &multisampleDepthIndex);
	//		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleDepthIndex);
	//		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_DEPTH_COMPONENT24, width, height, GL_TRUE);
	//		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	//		glBindFramebuffer(GL_FRAMEBUFFER, multisampleFbo);
	//		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, multisampleDepthIndex, 0);
	//		internalMultisampleDepthTexure.assign(width, height, 1, { GL_CLAMP, GL_CLAMP,
	//			GL_NEAREST, GL_NEAREST, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24 }, multisampleDepthIndex, GL_TEXTURE_2D_MULTISAMPLE);
	//	}
	//	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	//		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	//}
	/*if (multisampleLevel == 0)
		return fbo;
	else
		return multisampleFbo;*/
}

void RenderTarget::addTexture(const string & name, Texture & texture, unsigned int mipLevel)
{
	auto iter = textures.find(name);
	if (iter == textures.end()) {
		unsigned int index = textureList.size();
		textures.insert(pair<string, int>(name, index));
		textureList.push_back({ index, name, mipLevel, &texture });
	}
	else {
		RTInfo& info = textureList[iter->second];
		info.texture = &texture;
		info.mipLevel = mipLevel;
	}
	inited = false;
}

Texture * RenderTarget::getTexture(const string & name)
{
	if (name == "depthMap")
		return depthTexure;
	auto re = textures.find(name);
	if (re == textures.end())
		return nullptr;
	return textureList[re->second].texture;
}

Texture * RenderTarget::getTexture(unsigned int index)
{
	if (index >= textureList.size())
		return nullptr;
	return textureList[index].texture;
}

unsigned int RenderTarget::getTextureCount()
{
	return textureList.size();
}

bool RenderTarget::setTextureMipLevel(const string & name, unsigned int mipLevel)
{
	auto iter = textures.find(name);
	if (iter == textures.end())
		return false;
	RTInfo& info = textureList[iter->second];
	if (info.mipLevel == mipLevel)
		return true;
	info.mipLevel = mipLevel;
	inited = false;
	return true;
}

bool RenderTarget::setTextureMipLevel(unsigned int index, unsigned int mipLevel)
{
	if (index >= textureList.size())
		return false;
	RTInfo& info = textureList[index];
	if (info.mipLevel == mipLevel)
		return true;
	info.mipLevel = mipLevel;
	inited = false;
	return true;
}

void RenderTarget::setMultisampleLevel(unsigned int level)
{
	if (multisampleLevel != level) {
		multisampleLevel = level;
		inited = false;
	}
}

unsigned int RenderTarget::getMultisampleLevel()
{
	return multisampleLevel;
}

void RenderTarget::setDepthTexture(Texture & depthTexture)
{
	this->depthTexure = &depthTexture;
	inited = false;
}

Texture2D * RenderTarget::getInternalDepthTexture()
{
	return depthIndex == 0 ? NULL : &internalDepthTexure;
}

void RenderTarget::setTexture(Material & mat)
{
	for (auto b = textureList.begin(), e = textureList.end(); b != e; b++) {
		mat.setTexture(b->name, *b->texture);
	}
	if (depthTexure != NULL)
		mat.setTexture("depthMap", *depthTexure);
}

void RenderTarget::SetMultisampleFrame()
{
	if (multisampleLevel != 0) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFbo);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		/*glReadBuffer(GL_DEPTH_ATTACHMENT);
		glDrawBuffer(GL_DEPTH_ATTACHMENT);
		glBlitFramebufferEXT(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);*/
		currentFbo = fbo;
	}
}

unsigned int RenderTarget::getFBO()
{
	return multisampleLevel == 0 ? fbo : multisampleFbo;
}

void RenderTarget::resize(unsigned int width, unsigned int height)
{
	if (isDefault())
		return;
	if (inited && this->width == width && this->height == height)
		return;
	this->width = width;
	this->height = height;
	if (fbo != 0) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		if (depthOnly) {
			unsigned int index = depthTexure->resize(width, height);
			if (index == 0)
				throw runtime_error("Bind depth texture failed");
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, index, 0);
		}
		else {
			if (textureList.empty()) {
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}
			else {
				int i = 0;
				unsigned int* mode = new unsigned int[textures.size()];
				for (auto b = textureList.begin(), e = textureList.end(); b != e && i < GL_MAX_COLOR_ATTACHMENTS; b++, i++) {
					unsigned int index = b->texture->resize(width, height);
					if (index == 0)
						throw runtime_error("Resize texture failed");
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, index, b->mipLevel);
					mode[i] = GL_COLOR_ATTACHMENT0 + i;
				}
				glDrawBuffers(textures.size(), mode);
				delete[] mode;
			}
		}
	}
	if (multisampleLevel != 0 && multisampleFbo != 0) {
		glBindFramebuffer(GL_FRAMEBUFFER, multisampleFbo);
		if (multisampleIndex != 0) {
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, multisampleIndex, 0);
			glDeleteTextures(1, &multisampleIndex);
			multisampleIndex = 0;
		}
		glGenTextures(1, &multisampleIndex);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleIndex);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisampleLevel, GL_RGB, width, height, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, multisampleIndex, 0);
	}
	if (withDepthStencil) {
		//if (rbo != 0) {
		//	glBindRenderbuffer(GL_RENDERBUFFER, 0);
		//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
		//	glDeleteRenderbuffers(1, &rbo);
		//	rbo = 0;
		//}
		//glGenRenderbuffers(1, &rbo);
		//glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		//glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisampleLevel, GL_DEPTH24_STENCIL8, width, height);
		////glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisampleLevel, GL_DEPTH_COMPONENT24, width, height);
		//glBindRenderbuffer(GL_RENDERBUFFER, 0);
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
		////glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
		if (depthIndex != 0) {
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
			glDeleteTextures(1, &depthIndex);
			depthIndex = 0;
			if (multisampleLevel != 0) {
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, multisampleFbo);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, 0, 0);
				glDeleteTextures(1, &multisampleDepthIndex);
				multisampleDepthIndex = 0;
			}
		}
		glGenTextures(1, &depthIndex);
		glBindTexture(GL_TEXTURE_2D, depthIndex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthIndex, 0);
		internalDepthTexure.assign(width, height, 1, { TW_Clamp, TW_Clamp,
			TF_Point, TF_Point, TIT_Depth }, depthIndex);
		if (multisampleLevel != 0) {
			glGenTextures(1, &multisampleDepthIndex);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleDepthIndex);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisampleLevel, GL_DEPTH_COMPONENT24, width, height, GL_TRUE);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, multisampleFbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, multisampleDepthIndex, 0);
			internalMultisampleDepthTexure.assign(width, height, 1, { TW_Clamp, TW_Clamp,
				TF_Point, TF_Point, TIT_Depth }, multisampleDepthIndex, GL_TEXTURE_2D_MULTISAMPLE);
		}
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget::blitFrom(const RenderTarget & renderTarget, GLbitfield bit)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, renderTarget.multisampleLevel == 0 ? renderTarget.fbo : renderTarget.multisampleFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glBlitFramebuffer(0, 0, renderTarget.width, renderTarget.height, 0, 0, width, height, bit, bit == GL_DEPTH_BUFFER_BIT ? GL_NEAREST : GL_LINEAR);
	currentFbo = fbo;
}
