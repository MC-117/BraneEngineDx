#pragma once

#include "DX12InputLayout.h"

class DX12GraphicPipelineState
{
public:
	struct ID
	{
		union
		{
			uint64_t value;
			struct {
				uint8_t inputLayout;
				uint8_t cull;
				uint8_t blend;
				uint8_t depth;
				uint8_t dsvFormat;
				uint8_t sampleCount;
				uint16_t programID;
			} info;
		} word0;
		union
		{
			uint64_t value;
			uint8_t rtvFormats[8];
		} word1;

		bool operator<(const ID& id) const;
	};

	DX12GraphicPipelineState();
	virtual ~DX12GraphicPipelineState();
	void reset();
	ComPtr<ID3D12PipelineState> get(ComPtr<ID3D12Device> device);
	const ID& getID() const;
	void release();

	bool isValid() const;

	void setRootSignature(ComPtr<ID3D12RootSignature> rootSignature);

	void setInputLayout(DX12InputLayoutType type);

	void setCullType(DX12CullType type);
	void setBlendFlags(Enum<DX12BlendFlags> flags);
	void setDepthFlags(Enum<DX12DepthFlags> flags);

	void setShader(const DX12ShaderProgramData& data);

	void setSampleCount(int count);
	void setDSVFormat(DXGI_FORMAT dsvFormat);
	void setRTVFormat(int count, DXGI_FORMAT* rtvFormat);
protected:
	ID id = {};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	DX12InputLayoutType inputLayoutType = DX12InputLayoutType::None;
	DX12CullType cullType = DX12CullType::Cull_Off;

	unsigned int shaderProgramID = 0;

	Enum<DX12BlendFlags> blendFlags = 0;
	Enum<DX12DepthFlags> depthFlags = 0;

	bool update = true;

	ComPtr<ID3D12PipelineState> pipelineState;
};

class DX12ComputePipelineState
{
public:
	typedef uint8_t ID;
	DX12ComputePipelineState();
	virtual ~DX12ComputePipelineState();
	void reset();
	ComPtr<ID3D12PipelineState> get(ComPtr<ID3D12Device> device);
	const ID& getID() const;
	void release();

	bool isValid() const;

	void setRootSignature(ComPtr<ID3D12RootSignature> rootSignature);
	void setShader(const DX12ShaderProgramData& data);
protected:
	ID id;

	D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
	unsigned int shaderProgramID = 0;

	bool update = true;

	ComPtr<ID3D12PipelineState> pipelineState;
};

