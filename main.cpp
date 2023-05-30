#include <SDL2/SDL.h>
#include <vector>
#include <array>
#include <complex>
#include <map>
#include <algorithm>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

using Im = std::complex<float>;
using namespace std::complex_literals;

Im f_avg;

Im c_n(const std::vector<Im> &f, int n)
{
    Im c;

    float dt = 1.f / f.size();
    for (size_t i = 0; i < f.size(); ++i)
    {
        float t = i * dt;
        float arg = -(float)n * 2.f * M_PI * t;
        Im tmp = (f[i] - f_avg) * Im(
            std::cos(arg),
            std::sin(arg)
        );
        c += tmp * dt;
    }

    return c;
}

void draw_circle(SDL_Renderer* rend, int cx, int cy, int r)
{
    for (int y = -r; y < r; ++y)
    {
        for (int x = -r; x < r; ++x)
        {
            if (std::abs(std::sqrt(x * x + y * y) - r) <= 1.f)
                SDL_RenderDrawPoint(rend, cx + x, cy + y);
        }
    }
}

SDL_Window *win;
SDL_Renderer *rend;

bool image_drawn = false;
bool mouse_down = false;
std::vector<Im> f;

int range = 30;
int N = 1 + range * 2;
std::vector<Im> vc(N);
bool vc_calculated = false;

float t = 0.f;

bool render_original_drawing = true;
std::vector<Im> fourier_drawing;

void run()
{
    bool running = true;
    SDL_Event evt;
#ifndef __EMSCRIPTEN__
    while (running)
#endif
    {
        while (SDL_PollEvent(&evt))
        {
            if (evt.type == SDL_QUIT)
                running = false;

            if (evt.type == SDL_MOUSEBUTTONDOWN)
            {
                image_drawn = false;
                vc_calculated = false;
                f.clear();
                fourier_drawing.clear();
                mouse_down = true;
                render_original_drawing = true;
            }

            if (evt.type == SDL_MOUSEBUTTONUP)
            {
                image_drawn = true;
                mouse_down = false;

                Im back = f.back();
                for (int i = 0; i < 100; ++i)
                {
                    f.emplace_back(back + (f[0] - back) / 100.f * (float)(i + 1));
                }

                for (const auto &p : f)
                    f_avg += p;
                f_avg /= f.size();
            }

            if (evt.type == SDL_MOUSEMOTION && mouse_down)
            {
                f.emplace_back(Im(evt.motion.x, evt.motion.y));
            }

            if (evt.type == SDL_KEYDOWN)
            {
                switch (evt.key.keysym.sym)
                {
                case SDLK_SPACE:
                    render_original_drawing = !render_original_drawing;
                    break;
                }
            }
        }

        if (image_drawn)
#ifdef __EMSCRIPTEN__
            t += 1.f / f.size();
#else
            t += 5.f / f.size();
#endif

        if (t >= 1.f)
        {
            t = 0.f;
            fourier_drawing.clear();
        }

        SDL_RenderClear(rend);

        SDL_SetRenderDrawColor(rend, 100, 100, 100, 255);

        if (render_original_drawing)
        {
            for (size_t i = 1; i < f.size(); ++i)
            {
                SDL_RenderDrawLineF(rend,
                    f[i].real(),
                    f[i].imag(),
                    f[i - 1].real(),
                    f[i - 1].imag());
            }
        }

        SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
        for (size_t i = 1; i < fourier_drawing.size(); ++i)
        {
            SDL_RenderDrawLineF(rend,
                fourier_drawing[i].real(),
                fourier_drawing[i].imag(),
                fourier_drawing[i - 1].real(),
                fourier_drawing[i - 1].imag());
        }

        if (image_drawn)
        {
            if (!vc_calculated)
            {
                for (int i = 0; i < N; ++i)
                    vc[i] = c_n(f, i - range);

                vc_calculated = true;
            }

            SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(rend, 240, 240, 255, 180);

            Im orig;
            for (int i = 0; i < N; ++i)
            {
                int n = i - range;
                float x = 2.f * M_PI * (float)n * t;
                Im next = orig + vc[i] * Im(
                    std::cos(x),
                    std::sin(x)
                );
                SDL_RenderDrawLineF(rend,
                    orig.real() + f_avg.real(),
                    orig.imag() + f_avg.imag(),
                    next.real() + f_avg.real(),
                    next.imag() + f_avg.imag());
                draw_circle(rend,
                    orig.real() + f_avg.real(),
                    orig.imag() + f_avg.imag(),
                    std::sqrt(vc[i].real() * vc[i].real() + vc[i].imag() * vc[i].imag())
                );
                orig = next;
            }

            fourier_drawing.emplace_back(orig + f_avg);
            SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_NONE);
        }

        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderPresent(rend);
    }
}

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    win = SDL_CreateWindow("Fourier draw",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        800, 800, SDL_WINDOW_SHOWN);
    rend = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(run, -1, 1);
#else
    run();
#endif

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

