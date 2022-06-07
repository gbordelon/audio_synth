# audio_synth
I created a Python script which uses mmap and pygame to play chunks of raw PCM frames. Now my synth can simply write raw 32-bit floating point PCM frames to the mmap'd file and the Python script will play it.

--

I like the feature list here: https://www.futur3soundz.com/xfm2
I started from https://m.cplusplus.com/forum/beginner/166954/#msg840873 and https://wavefilegem.com/how_wave_files_work.html
I also used ffmpeg to convert an audio file to wav so I could compare the file headers.
I can use VLC to play the generated wave files.
