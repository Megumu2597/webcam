import subprocess
import numpy as np
#https://superkogito.github.io/blog/2020/03/19/ffmpeg_pipe.html
# init command
mp3_path = "udp://127.0.0.1:1025"
ffmpeg_command = ["ffmpeg", "-i", mp3_path, 
                "-f", "mpegts", "pipe:1"]
"""["ffmpeg", "-i", mp3_path,
                "-ab", "128k", "-acodec", "pcm_s16le", "-ac", "0", "-ar", target_fs, "-map",
                "0:a", "-map_metadata", "-1", "-sn", "-vn", "-y",
                "-f", "wav", "pipe:1"] """

# excute ffmpeg command
pipe = subprocess.run(ffmpeg_command,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    bufsize=10**8)

# debug
print("!!!!\n",pipe.stdout, "\n 888888888888 \n",pipe.stderr,"\n!!!!!!\n")

# read signal as numpy array and assign sampling rate
audio_np = np.frombuffer(buffer=pipe.stdout, dtype=np.uint16, offset=8*44)
sig, fs  = audio_np, target_fs