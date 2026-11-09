#include <algorithm>
#include <vector>
#include <cstdio>
#include <ctime>

void init(std::vector<int>* vp) {
        vp->clear();
        for (int i = 0; i < 7; i++) {
                for (int j = 0; j < 10; j++)
                        vp->push_back(i);
        }
        std::random_shuffle(vp->begin(), vp->end());
}

int main(int argc, char *argv[]) {
        std::vector<int> v;
        int n = 1000;
        int uniq = 0;

        std::srand(std::time(nullptr));
        if (argc > 1)
                n = atoi(argv[1]);

        for (int i = 0; i < n; i++) {
                init(&v);
                // draw ten.
                int colors[7] = {};
                for (int j = 0; j < 10; j++) {
                        colors[v[j]]++;
                }

                // count.
                for (int j = 0; j < 7; j++) {
                        if (colors[j])
                                uniq++;
                }
        }
        printf("%lu %lu %f\n", uniq, n, (1.0 * uniq)/n);
}
