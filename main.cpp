#include <SDL2/SDL.h>
#include <vector>
#include <array>
#include <complex>

#define F_N 1000

using Im = std::complex<float>;
using namespace std::complex_literals;

std::array<Im, F_N> f_t()
{
    std::array<Im, F_N> f;
    for (int i = 0; i < F_N; ++i)
    {
        f[i].real((float)i / 10.f);
        f[i].imag(10.f * std::sin(f[i].real()));
    }
    return f;
}

Im c_n(const std::array<Im, F_N> &f, int n)
{
    Im c;

    float dt = 1.f / F_N;
    for (int i = 0; i < F_N; ++i)
    {
        float t = i * dt;
        float arg = -(float)n * 2.f * M_PI * t;
        Im tmp = f[i] * Im(
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

    std::array<Im, F_N> f = f_t();

    int range = 50;
    int N = 1 + range * 2;
    std::vector<Im> vc(N);

    for (int i = 0; i < N; ++i)
        vc[i] = c_n(f, i - range);

    float t = 0.f;

    bool running = true;
    SDL_Event evt;
    while (running)
    {
        while (SDL_PollEvent(&evt))
        {
            if (evt.type == SDL_QUIT)
                running = false;
        }

        t += 0.001f;

        SDL_RenderClear(rend);

        SDL_SetRenderDrawColor(rend, 100, 100, 100, 255);

        for (size_t i = 1; i < f.size(); ++i)
        {
            SDL_RenderDrawLineF(rend,
                f[i].real() * 10.f + 0.f,
                f[i].imag() * 10.f + 300.f,
                f[i - 1].real() * 10.f + 0.f,
                f[i - 1].imag() * 10.f + 300.f);
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
                orig.real() * 10.f + 0.f,
                orig.imag() * 10.f + 300.f,
                next.real() * 10.f + 0.f,
                next.imag() * 10.f + 300.f);
            orig = next;
        }

        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderPresent(rend);
    }

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

