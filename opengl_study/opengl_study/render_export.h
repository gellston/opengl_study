#pragma once

#ifndef RENDER_EXPORT_HEADER
#define RENDER_EXPORT_HEADER

#ifndef RENDER_EXPORT
#define RENDER_API __declspec(dllimport)
#else
#define RENDER_API __declspec(dllexport)
#endif

#endif