This is the access code to the underlying sound library.  It was made to make interfacing with OpenAL less obnoxious.

This code just needs to be compiled into an object file, as it is used elsewhere.

openal_test.c is similar to an earlier OpenAL demo, however it uses the sound library to accomplish the demo.  It works as follows:
1: The user selects a .wav file (anything that exists in ~/projectDaredevil/sound should work, like test.wav)
2: the sound begins playing
3: the user may move the sound around them in 2-dimensional space by pressing W, A, S, or D, and enter (they may also "rotate" left / right by pressing Q and E).

WARNING: LIBOPENAL.SO SHOULD NOT TECHNICALLY BE HERE, AND SHOULD BE REPLACED WITH THE INSTALLED VERSION WHICH WOULD BE IN:
/usr/local/lib/libopenal.so