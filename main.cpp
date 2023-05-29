#include <SDL2/SDL.h>
#include <vector>
#include <array>
#include <complex>

using Im = std::complex<float>;
using namespace std::complex_literals;

Im f_avg;

Im c_n(const std::vector<Im> &f, int n)
{
    Im c;

    float dt = 1.f / f.size();
    for (int i = 0; i < f.size(); ++i)
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

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("Fourier draw",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        600, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer *rend = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    bool image_drawn = false;
    bool mouse_down = false;
    std::vector<Im> f;

    int range = 100;
    int N = 1 + range * 2;
    std::vector<Im> vc(N);
    bool vc_calculated = false;

    float t = 0.f;

    bool running = true;
    SDL_Event evt;
    while (running)
    {
        while (SDL_PollEvent(&evt))
        {
            if (evt.type == SDL_QUIT)
                running = false;

            if (evt.type == SDL_MOUSEBUTTONDOWN)
                mouse_down = true;
            if (evt.type == SDL_MOUSEBUTTONUP)
            {
                image_drawn = true;
                mouse_down = false;

                for (const auto &p : f)
                    f_avg += p;
                f_avg /= f.size();
            }

            if (evt.type == SDL_MOUSEMOTION && mouse_down)
            {
                f.emplace_back(Im(evt.motion.x, evt.motion.y));
            }
        }

        if (image_drawn)
            t += 5.f / f.size();

        SDL_RenderClear(rend);

        SDL_SetRenderDrawColor(rend, 100, 100, 100, 255);

        for (size_t i = 1; i < f.size(); ++i)
        {
            SDL_RenderDrawLineF(rend,
                f[i].real(),
                f[i].imag(),
                f[i - 1].real(),
                f[i - 1].imag());
        }

        if (image_drawn)
        {
            if (!vc_calculated)
            {
                for (int i = 0; i < N; ++i)
                    vc[i] = c_n(f, i - range);

                vc_calculated = true;
            }

            SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);

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
                orig = next;
            }
        }

        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderPresent(rend);
    }

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

