#### NOTE ####
This is just a testing/sample usage.
Development shifting to: https://github.com/cruxeon/ffenc.git

--------------------------------------------------------------
### FFMPEG Transcoding to Add User SEI Data ###
Modified version of the FFMPEG transcoding sample to have multiple output streams and encode user SEI message in each stream.

#### Usage ####
Transcoding input video with custom SEI and transmitting:
> ./test \<input_video> \<output_location>

- input_video: location of video (= sample.mp4)
- output_location: either a filename or rtsp url (url = rtsp://localhost:8888/live.sdp)

Receiving Transmitted Video:
> ffplay -rtsp_flags listen -i rtsp://localhost:8888/live.sdp

To view rtsp stream (if necessary)

