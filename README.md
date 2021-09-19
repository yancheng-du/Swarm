# Swarm!

To build and run the old version on macOS:
1. Install [Xcode](https://apps.apple.com/us/app/xcode/id497799835) via App Store
2. Install Xcode command line tools via terminal with `xcode-select —install`
3. Install [homebrew](https://brew.sh) via terminal with `/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"`
4. Install glfw with `brew install glfw`
5. Install glm with `brew install glm`
6. Install libfreenect with `brew install libfreenect`
7. Install opencv with `brew install opencv`
8. In oldsrc directory `make`
9. In oldsrc directory `./main`

## Using git pull
tutorial source here: https://git-lfs.github.com/
1. Install git lfs using homebrew or from the link above
2. On command line use `git lfs install`
3. Now you should use `git pull` as normal
