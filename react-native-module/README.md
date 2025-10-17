# react-native-sss

React Native Seven Segment Scan

## What is this

This is part of the code produced for a mastersthesis. No working method was produced, but this repository might yet contain interesting results.

This module is supposed to be used in a React-Native application, but the C++ code is usable by itself.

>>> Major Bug: No configuration to allow access to the .xml file containing the cascade classifier. Thus the module will not worked until this is fixed.

## Installation

On one cmd start local registry

```sh
verdaccio
```

Then, do the following:

>>> Increment the version number in package.json

Then, on another cmd, publish the module  

```sh
npm set registry http://localhost:4873/
npm publish --registry http://localhost:4873/
```

Finally, you should be able to update the package in the project.
If not installed yet, install with (in project dir):

```sh
npm install --registry http://localhost:4873 react-native-sss
```

```sh
npm uninstall --registry http://localhost:4873 react-native-sss
```

## Usage


```js
import { multiply } from 'react-native-sss';

// ...

const result = multiply(3, 7);
```


## Contributing

See the [contributing guide](CONTRIBUTING.md) to learn how to contribute to the repository and the development workflow.

## Project structure

The functional code is located in `cpp`

- `cpp/jsi`: **J**ava**S**cript **I**nterface folder from fast-opencv
- `cpp/fast-opencv`: Files (mostly) copied from fast-opencv, mostly dealing with conversions between JS and CPP data
- `cpp/main`: Contains the original functionality of this project. (Should be able to run as an independent cpp file)


## Referenced Code

Foundation of this project is based on Łukasz Kurant's fast openCV plugin: https://github.com/lukaszkurantdev/react-native-fast-opencv

https://blog.notesnook.com/getting-started-react-native-jsi/

## License

MIT

---

Made with [create-react-native-library](https://github.com/callstack/react-native-builder-bob)
