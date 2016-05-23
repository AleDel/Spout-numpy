#ifdef _DEBUG
#define _DEBUG_WAS_DEFINED
#undef _DEBUG
#endif

#include <Python.h>

#ifdef _DEBUG_WAS_DEFINED
#define _DEBUG 1
#endif

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <string>
#include "Spout.h"


/////////////////////// pybind11
namespace py = pybind11;

class Spoutnumpy {
public:
	//Spout(const std::string &name, std::vector<int> img) : name(name), img(img) { }
	
	void setName(const std::string &name_) { name = name_; }
	
	const std::string &getName() const { return name; }
	
	bool send(const std::string name, py::array_t<uint8_t, py::array::c_style> cvimage) {
		
		auto buffer = cvimage.request();
		
		if (buffer.ndim != 3 ) {
			//throw std::runtime_error("Number of dimensions must be 3");	
			return 0;
		}

		unsigned int w = buffer.shape[1];
		unsigned int h = buffer.shape[0];

		
		if (g_D3D11Device == nullptr) {
			g_D3D11Device = sdx.CreateDX11device();
			g_D3D11Device->GetImmediateContext(&g_pImmediateContext);
		}

		if (sendingTexture == NULL) {

			sdx.CreateSharedDX11Texture(g_D3D11Device, w, h, DXGI_FORMAT_B8G8R8A8_UNORM, &sendingTexture, sharedSendingHandle);

		}

		// 3 channels
		if (buffer.shape[2] == 3) {
			
			// add channel alpha
			uint8_t* rgba = new uint8_t[w*h * 4];
			uint8_t* rgb = reinterpret_cast<uint8_t*>(buffer.ptr);
			int e = 0;
			for (int i = 0; i < w*h * 4; i++) {
				if (i % 4 == 3) {
					rgba[i] = 255;
					e += 1;
				}
				else {
					rgba[i] = rgb[i - e];
				}

			}
			//memcpy(rgba, rgb, w);
			
			g_pImmediateContext->UpdateSubresource(sendingTexture, 0, NULL, rgba, w * 4, 0);
			
			free(rgba);
		}
		else {
			g_pImmediateContext->UpdateSubresource(sendingTexture, 0, NULL, buffer.ptr, w*buffer.shape[2], 0);
		}
		
		
		g_pImmediateContext->Flush();

		spoutsendernames.CreateSender(name.c_str(), w, h, sharedSendingHandle);
		
		return 1;
	}

	py::array_t<uint8_t, py::array::c_style> receive(const std::string name) {
		if (g_D3D11Device == nullptr) {
			g_D3D11Device = sdx.CreateDX11device();
			g_D3D11Device->GetImmediateContext(&g_pImmediateContext);
		}
		unsigned int w, h = 64;
		HANDLE sharedHandle = NULL;
		DWORD dwFormat;
		uint8_t* aasource = nullptr;
		//BYTE* aasource = nullptr;
		if (spoutsendernames.GetSenderInfo(name.c_str(), w, h, sharedHandle, dwFormat)) {
		//if ( spoutsendernames.FindSender((char*)name.c_str(), w, h, sharedHandle, dwFormat) ) {
			//printf("CreateInterop - %dx%d - dwFormat (%d) \n", w, h, dwFormat);
			ID3D11Resource * tempResource11 = nullptr;
			HRESULT openResult = g_D3D11Device->OpenSharedResource(sharedHandle, __uuidof(ID3D11Resource), (void**)(&tempResource11));
			
			D3D11_RESOURCE_DIMENSION aa;
			tempResource11->GetType(&aa);
			//printf("tipo de recurso: %d", aa);
			
			ID3D11Texture2D* tex = (ID3D11Texture2D*)tempResource11;
			

			D3D11_TEXTURE2D_DESC description;
			tex->GetDesc(&description);
			description.BindFlags = 0;
			description.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			description.Usage = D3D11_USAGE_STAGING;

			ID3D11Texture2D* texTemp = NULL;

			HRESULT hr = g_D3D11Device->CreateTexture2D(&description, NULL, &texTemp);
			if (FAILED(hr))
			{
				if (texTemp)
				{
					texTemp->Release();
					texTemp = NULL;
				}
				return NULL;
			}
			g_pImmediateContext->CopyResource(texTemp, tex);


			D3D11_MAPPED_SUBRESOURCE  mapResource;
			g_pImmediateContext->Map(texTemp, 0, D3D11_MAP_READ, NULL, &mapResource);

			//printf("mapResource.RowPitch: %x", mapResource.RowPitch);

			const int pitch = mapResource.RowPitch;
			uint8_t* source = (uint8_t*)(mapResource.pData);
			uint8_t* dest = new uint8_t[(w)*(h) * 4];
			
			uint8_t* destTemp = dest;
			for (int i = 0; i < h; ++i)
			{
				memcpy(destTemp, source, w * 4);
				source += pitch;
				destTemp += w * 4;
			}
			
			aasource = dest;
			
			/*//aasource = reinterpret_cast<uint8_t*>(dest);*/
			g_pImmediateContext->Unmap(texTemp, 0);
			texTemp->Release();
			texTemp = NULL;
			

		}
		else {
			printf("not found sender named: %s", name);
		}

		/*
		uint8_t* rgba = new uint8_t[w*h * 4];
		
		int e = 0;
		for (int i = 0; i < w*h * 4; i++) {
			switch (i%4)
			{
			case 0:
				rgba[i] = 255;
				break;
			case 1:
				rgba[i] = 0;
				break;
			case 2:
				rgba[i] = 0;
				break;
			case 3:
				rgba[i] = 255;
				break;
			}

		}*/
		auto result = py::array(py::buffer_info(
			aasource,            /* Pointer to data (nullptr -> ask NumPy to allocate!) */
			sizeof(uint8_t),     /* Size of one item */
			py::format_descriptor<uint8_t>::value, /* Buffer format */
			3,//buf1.ndim,          /* How many dimensions? */
			{ h, w, 4},  /* Number of elements for each dimension */
			{ sizeof(uint8_t)*w*4, sizeof(uint8_t)*4 , sizeof(uint8_t)} /* Strides for each dimension */
		));
		free(aasource);
		
		return result;
	}
	
	std::string name;
	py::array_t<uint8_t, py::array::c_style> cvimage;

private:

	ID3D11Device* g_D3D11Device;
	ID3D11DeviceContext* g_pImmediateContext = NULL;
	HANDLE sharedSendingHandle = NULL;
	ID3D11Texture2D * sendingTexture = NULL;
	spoutSenderNames spoutsendernames;
	spoutDirectX sdx;
};



PYBIND11_PLUGIN(spout) {
	py::module m("spout", "Spout plugin");

	py::class_<Spoutnumpy>(m, "Spoutnumpy")
		//.def(py::init<const std::string &, std::vector<int>>())
		.def(py::init<>())
		.def_readwrite("name", &Spoutnumpy::name)
		.def_readwrite("cvimage", &Spoutnumpy::cvimage)
		.def("setName", &Spoutnumpy::setName)
		.def("getName", &Spoutnumpy::getName)
		.def("send", &Spoutnumpy::send, "A function which test Spout sender spout")
		.def("receive",&Spoutnumpy::receive)
		.def("__repr__",
			[](const Spoutnumpy &a) {
				return "<spout '" + a.name + "'>";
			}
		);

	return m.ptr();
}