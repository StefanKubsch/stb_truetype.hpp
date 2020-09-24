/*
********************************************************************
* stb_truetype.hpp example program                                 *
*                                                                  *
* (C) 2019 - present by Stefan Kubsch                              *
********************************************************************
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#define NOMINMAX
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <fstream>
#include <istream>
#include <string>

// Include "lwmf.hpp"
#define LWMF_LOGGINGENABLED
#include "./lwmf/lwmf.hpp"

// Include stb_truetype.hpp
#define STB_TRUETYPE_IMPLEMENTATION
#include "./src/stb_truetype.hpp"

// "ScreenTexture" is the main render target in our demo!
inline lwmf::TextureStruct ScreenTexture{};

class GFX_TextClass final
{
public:
	void InitFont(const std::string& FontName, std::int_fast32_t FontSize, std::int_fast32_t Color);
	void RenderText(const std::string& Text, std::int_fast32_t x, std::int_fast32_t y);

private:
	struct GlyphStruct final
	{
		std::int_fast32_t Height{};
		std::int_fast32_t Width{};
		std::int_fast32_t Advance{};
		std::int_fast32_t Baseline{};
		GLuint Texture{};
	};

	lwmf::ShaderClass GlyphShader{};
	std::vector<GlyphStruct> Glyphs{};

	std::int_fast32_t FontHeight{};
};

inline void GFX_TextClass::InitFont(const std::string& FontName, const std::int_fast32_t FontSize, const std::int_fast32_t Color)
{
	GlyphShader.LoadShader("Default", ScreenTexture);

	// Get raw (binary) font data
	std::ifstream FontFile(FontName.c_str(), std::ifstream::binary);
	FontFile.seekg(0, std::ios::end);
	std::vector<unsigned char> FontBuffer(FontFile.tellg());
	FontFile.seekg(0, std::ios::beg);
	FontFile.read(reinterpret_cast<char*>(FontBuffer.data()), FontBuffer.size());

	// Render the glyphs for ASCII chars
	// This makes 128 chars, the last 96 are printable (from 32 = "space" on)

	stbtt_fontinfo FontInfo{};
	stbtt_InitFont(FontInfo, FontBuffer, 0);

	std::int_fast32_t Width{};
	FontHeight = FontSize + 1;
	const std::int_fast32_t Height{ FontHeight + 5 };
	constexpr std::int_fast32_t LastASCIIChar{ 127 }; // All ASCII chars except DEL (127)

	for (char Char{}; Char < LastASCIIChar; ++Char)
	{
		lwmf::IntPointStruct i0{};
		lwmf::IntPointStruct i1{};

		stbtt_GetCodepointBitmapBox(FontInfo, Char, 1.0F, 1.0F, i0.X, i0.Y, i1.X, i1.Y);
		Width += 1 + (i1.X * FontSize / 1000) + 1 - (i0.X * FontSize / 1000);
	}

	const std::size_t Size{ static_cast<std::size_t>(Width) * static_cast<std::size_t>(Height) };
	std::vector<unsigned char> BakedFontGreyscale(Size);
	std::vector<stbtt_bakedchar> CharData(LastASCIIChar);
	stbtt_BakeFontBitmap(FontBuffer, 0, static_cast<float>(FontSize), BakedFontGreyscale, Width, Height, 0, LastASCIIChar, CharData);

	// Since the glyphs were rendered in greyscale, they need to be colored...
	const lwmf::ColorStruct TempColor{ lwmf::INTtoRGBA(Color) };
	std::vector<std::int_fast32_t> FontColor(Size);

	for (std::size_t i{}; i < BakedFontGreyscale.size(); ++i)
	{
		FontColor[i] = lwmf::RGBAtoINT(TempColor.Red, TempColor.Green, TempColor.Blue, BakedFontGreyscale[i]);
	}

	Glyphs.resize(LastASCIIChar);

	for (std::int_fast32_t Char{}; Char < LastASCIIChar; ++Char)
	{
		lwmf::FloatPointStruct QuadPos{};
		stbtt_aligned_quad Quad{};
		stbtt_GetBakedQuad(CharData, Width, Height, Char, QuadPos.X, QuadPos.Y, Quad, 1);

		const lwmf::IntPointStruct Pos{ static_cast<std::int_fast32_t>(Quad.s0 * Width), static_cast<std::int_fast32_t>(Quad.t0 * Height) };
		Glyphs[Char].Width = static_cast<std::int_fast32_t>(((Quad.s1 - Quad.s0) * Width) + 1.0F);
		Glyphs[Char].Height = static_cast<std::int_fast32_t>(((Quad.t1 - Quad.t0) * Height) + 1.0F);
		Glyphs[Char].Advance = static_cast<std::int_fast32_t>(std::roundf(QuadPos.X));
		Glyphs[Char].Baseline = static_cast<std::int_fast32_t>(-Quad.y0);

		// Blit single glyphs to individual textures
		lwmf::TextureStruct TempGlyphTexture{};
		TempGlyphTexture.Pixels.resize(static_cast<std::size_t>(Glyphs[Char].Width) * static_cast<std::size_t>(Glyphs[Char].Height));
		TempGlyphTexture.Width = Glyphs[Char].Width;
		TempGlyphTexture.Height = Glyphs[Char].Height;

		for (std::int_fast32_t DestY{}, SrcY{ Pos.Y }; SrcY < Pos.Y + Glyphs[Char].Height; ++SrcY, ++DestY)
		{
			const std::int_fast32_t TempDestY{ DestY * TempGlyphTexture.Width };
			const std::int_fast32_t TempSrcY{ SrcY * Width };

			for (std::int_fast32_t DestX{}, SrcX{ Pos.X }; SrcX < Pos.X + Glyphs[Char].Width; ++SrcX, ++DestX)
			{
				TempGlyphTexture.Pixels[TempDestY + DestX] = FontColor[TempSrcY + SrcX];
			}
		}

		GlyphShader.LoadTextureInGPU(TempGlyphTexture, &Glyphs[Char].Texture);
	}
}

inline void GFX_TextClass::RenderText(const std::string& Text, std::int_fast32_t x, const std::int_fast32_t y)
{
	for (const char& Char : Text)
	{
		GlyphShader.RenderTexture(&Glyphs[Char].Texture, x, y - Glyphs[Char].Baseline + FontHeight, Glyphs[Char].Width, Glyphs[Char].Height, true, 1.0F);
		x += Glyphs[Char].Advance;
	}
}

std::int_fast32_t WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nShowCmd);

	lwmf::WindowInstance = hInstance;

	// Create window and OpenGL context
	lwmf::CreateOpenGLWindow(lwmf::WindowInstance, ScreenTexture, 1280, 720, "stb_truetype.hpp example - press ESC to exit!", false);
	// Set VSync: 0 = off, -1 = on (adaptive vsync = smooth as fuck)
	lwmf::SetVSync(0);
	// Load OpenGL/wgl extensions
	lwmf::InitOpenGLLoader();
	// Check for SSE
	lwmf::CheckForSSESupport();
	// Init raw devices
	lwmf::RegisterRawInputDevice(lwmf::MainWindow, lwmf::DeviceIdentifier::HID_KEYBOARD);

	// Inital clearance of window. Looks better while loading the rest of the game...
	lwmf::ClearTexture(ScreenTexture, 0x00000000);
	lwmf::ClearBuffer();
	lwmf::SwapBuffer();

	// Define some test strings
	const std::string Text1{ "abcdefghijklmnopqrstuvwxyz" };
	const std::string Text2{ "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
	const std::string Text3{ "0123456789!?$&,.;:#+*=-_/()[]{}" };

	// Init fonts
	GFX_TextClass TestFont1{};
	TestFont1.InitFont("./Fonts/Ubuntu_Mono/UbuntuMono-Bold.ttf", 70, 0xFF0000FF);

	GFX_TextClass TestFont2{};
	TestFont2.InitFont("./Fonts/Ubuntu_Mono/UbuntuMono-Bold.ttf", 10, 0xFFFFFFFF);

	GFX_TextClass TestFont3{};
	TestFont3.InitFont("./Fonts/Mansalva/Mansalva-Regular.ttf", 70, 0xFFFF0000);

	GFX_TextClass TestFont4{};
	TestFont4.InitFont("./Fonts/Mansalva/Mansalva-Regular.ttf", 20, 0xFFFFFFFF);

	bool Quit{};

	while (!Quit)
	{
		MSG Message{};

		while (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
		{
			if (Message.message == WM_QUIT)
			{
				Quit = true;
				break;
			}

			DispatchMessage(&Message);
		}

		lwmf::ClearBuffer();

		TestFont1.RenderText(Text1, 10, 0);
		TestFont1.RenderText(Text2, 10, 70);
		TestFont1.RenderText(Text3, 10, 140);

		TestFont2.RenderText(Text1, 10, 250);
		TestFont2.RenderText(Text2, 10, 270);
		TestFont2.RenderText(Text3, 10, 290);

		TestFont3.RenderText(Text1, 10, 320);
		TestFont3.RenderText(Text2, 10, 400);
		TestFont3.RenderText(Text3, 10, 480);

		TestFont4.RenderText(Text1, 10, 600);
		TestFont4.RenderText(Text2, 10, 620);
		TestFont4.RenderText(Text3, 10, 640);

		Sleep(10);
		lwmf::SwapBuffer();
	}

	lwmf::UnregisterRawInputDevice(lwmf::DeviceIdentifier::HID_KEYBOARD);
	lwmf::DeleteOpenGLContext();

	return EXIT_SUCCESS;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INPUT:
		{
			RAWINPUT RawDev{};
			UINT DataSize{ sizeof(RAWINPUT) };
			UINT HeaderSize{ sizeof(RAWINPUTHEADER) };
			HRAWINPUT Handle{ reinterpret_cast<HRAWINPUT>(lParam) };
			GetRawInputData(Handle, RID_INPUT, &RawDev, &DataSize, HeaderSize);

			switch (RawDev.header.dwType)
			{
				case RIM_TYPEKEYBOARD:
				{
					if (RawDev.data.keyboard.Message == WM_KEYDOWN || RawDev.data.keyboard.Message == WM_SYSKEYDOWN)
					{
						switch (RawDev.data.keyboard.VKey)
						{
							case VK_ESCAPE:
							{
								PostQuitMessage(0);
								break;
							}
							default: {}
						}
					}
					break;
				}
				default: {}
			}
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		default: {}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}