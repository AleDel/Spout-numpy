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
		
		// only with 3 dimensions 
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
		.def("__repr__",
			[](const Spoutnumpy &a) {
				return "<spout '" + a.name + "'>";
			}
		);

	return m.ptr();
}