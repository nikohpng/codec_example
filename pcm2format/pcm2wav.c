#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LAYOUT_MONO   1 << 0
#define LAYOUT_STEREO 1 << 2
#define FMT_PCM        1
#define FMT_IEEE_FLT   3
#define FMT_ALAW       6
#define FMT_ULAW       7

typedef struct {
    unsigned char   chunk_id[4];        // RIFF string
    unsigned int    chunk_size;         // overall size of file in bytes (36 + data_size)
    unsigned char   sub_chunk1_id[8];   // WAVEfmt string with trailing null char
    unsigned int    sub_chunk1_size;    // 16 for PCM.  This is the size of the rest of the Subchunk which follows this number.
    unsigned short  audio_format;       // format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
    unsigned short  num_channels;       // Mono = 1, Stereo = 2
    unsigned int    sample_rate;        // 8000, 16000, 44100, etc. (blocks per second)
    unsigned int    byte_rate;          // SampleRate * NumChannels * BitsPerSample/8
    unsigned short  block_align;        // NumChannels * BitsPerSample/8
    unsigned short  bits_per_sample;    // bits per sample, 8- 8bits, 16- 16 bits etc
    unsigned char   sub_chunk2_id[4];   // Contains the letters "data"
    unsigned int    sub_chunk2_size;    // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
} wav_header_t;

char* dummy_get_raw_pcm (char *p, int *bytes_read)
{
	long lSize;
	char *pcm_buf;
	size_t result;
	FILE *fp_pcm;
	
	fp_pcm = fopen (p, "rb");
	if (fp_pcm == NULL) {
		printf ("File error");
		exit (1);
	}
	
	// obtain file size:
	fseek (fp_pcm , 0 , SEEK_END);
	lSize = ftell (fp_pcm);
	rewind (fp_pcm);
	
	// allocate memory to contain the whole file:
	pcm_buf = (char*) malloc (sizeof(char) * lSize);
	if (pcm_buf == NULL) {
		printf ("Memory error");
		exit (2);
	}
	
	// copy the file into the pcm_buf:
	result = fread (pcm_buf, 1, lSize, fp_pcm);
	if (result != lSize) {
		printf ("Reading error");
		exit (3);
	}
	
	*bytes_read = (int) lSize;
	return pcm_buf;	
}

void create_wav_header(int raw_sz, int sample_rate, int channles, int audio_format, int bits_per_sample, wav_header_t *wh)
{
	// RIFF chunk
    memcpy(wh->chunk_id, "RIFF", strlen("RIFF"));
	wh->chunk_size = 44 + raw_sz - 8;
	
	// fmt sub-chunk (to be optimized)
	memcpy(wh->sub_chunk1_id, "WAVEfmt ", strlen("WAVEfmt "));
	wh->sub_chunk1_size = 2 << 3;
	wh->audio_format = audio_format;
	wh->num_channels = channles;
	wh->sample_rate = sample_rate;
	wh->bits_per_sample = bits_per_sample;
	wh->block_align = wh->num_channels * wh->bits_per_sample / 8;
	wh->byte_rate = wh->sample_rate * wh->num_channels * wh->bits_per_sample / 8;
	
	// data sub-chunk
	memcpy(wh->sub_chunk2_id, "data", strlen("data"));
	wh->sub_chunk2_size = raw_sz;
}

void dump_wav_header (wav_header_t *wh)
{
	printf ("=========================================\n");
	printf ("chunk_id:\t\t\t%s\n", wh->chunk_id);
	printf ("chunk_size:\t\t\t%d\n", wh->chunk_size);
	printf ("sub_chunk1_id:\t\t\t%s\n", wh->sub_chunk1_id);
	printf ("sub_chunk1_size:\t\t%d\n", wh->sub_chunk1_size);
	printf ("audio_format:\t\t\t%d\n", wh->audio_format);
	printf ("num_channels:\t\t\t%d\n", wh->num_channels);
	printf ("sample_rate:\t\t\t%d\n", wh->sample_rate);
	printf ("bits_per_sample:\t\t%d\n", wh->bits_per_sample);
	printf ("block_align:\t\t\t%d\n", wh->block_align);
	printf ("byte_rate:\t\t\t%d\n", wh->byte_rate);
	printf ("sub_chunk2_id:\t\t\t%s\n", wh->sub_chunk2_id);
	printf ("sub_chunk2_size:\t\t%d\n", wh->sub_chunk2_size);
	printf ("=========================================\n");
}

int main (int argc, char* argv[])
{
	int raw_sz = 0;
	FILE *fwav;
	wav_header_t wheader;
	
	memset (&wheader, '\0', sizeof (wav_header_t));
	
	// dummy raw pcm data
	char *pcm_buf = dummy_get_raw_pcm ("./assets/number.pcm", &raw_sz);
	create_wav_header(0,8000, LAYOUT_MONO, FMT_PCM, 16, &wheader);
	
	// 输出PCM数据
	fwav = fopen("./out/number.wav", "wb");
	fwrite(&wheader, 1, sizeof(wheader), fwav);
	fwrite(pcm_buf, 1, raw_sz, fwav);

    // 刷新头中文件大小
    wheader.chunk_size = raw_sz + wheader.chunk_size;
    wheader.sub_chunk2_size = raw_sz;
    fseek(fwav, 0 , SEEK_SET);
    fwrite(&wheader, 1, sizeof(wheader), fwav);
    dump_wav_header (&wheader);

    // 关闭文件并释放资源
	fclose(fwav);
	if (pcm_buf)
		free (pcm_buf);
	
	return 0;
}