#include "file_io.h"

// Test cases:
// char fileName[] = "..\\implant\\abc.txt"
// char fileName[] = "..\\xyz.txt";
void testReadFileContent() {
	FileIO fileIO = FileIO();
	char fileName[] = "..\\implant\\abc.txt"; // TODO: Make sure this file exists to perform this test
	fileIO.ReadFileContent(fileName);
}

void testDownloadFileViaUrl() {
	FileIO fileIO = FileIO();
	char url[] = "https://154.82.111.116.sslip.io/dl2/P7HdDAIhXo6gnaj2fSgnunmqAWqLljyk1bd2JOX7-ydU7BOrvipc5zjbA0rjfeeISwNuRuVBpPwyv0LxiuDOQ-3q0lft9k0qkPMxvKgLHOPJQ1KP6NHyyjfcDcZuP7Qyyj877lVdZB-OFvyhZ2T9_DRAyztVrvkEae5JKTmXe-kxzOK1dGBRaF38DZ7yPblIZT6iVVE6qDgGuxIWejK-nVliwonVpHq4uvxILoPVkrVZE-NlwXcoeZgMwgJmcWWjg7F7fmej0_z3kK81UOkIpMQUVKt1ZEx_z5narfUjvaaHBtHWjjd-0mEG6DZVUCdRn449u-tbOB_QSpRvEEfuyTcoy5bFPKFewzw8vRNaoBiZbJxabpT5ddZjywV6ps82g5WnK5uZH9G0PJuohDo5IaPkCDy3HcOJnzMCzi3oo3-txB4dEIS7RfM3-EceWHAfGXaF2iObUP3FFzF_3fsQseDOO-ZwFrno91soAuCcqZfLFd4y7xdmYrwAliXqUg05oQ0pfsQ9pijW4XIp5C73ZqOj7f7P14_4cwWHjXeec-kIAWgZ-LbZshPfeFVCtL8A8b0oZeA20_teFdjw2VwYCyQb11VtQoYTrwsifXGV3cmpLgv7VJVO4FVvRZFxvagwiEOCjl765-8ylvJzYoCWBZyRA1DFXbgAopaNcJ_SEbgRdTgHdOa-ZCYQfydc-op9GH48J14OAhkGEAkZEdNaVIlVeBjCztCwzT5bNNBq6i1VwbsUspAc3Zy5Avug2Cfb8OF33nab-ZRFzpbmvN1svrtNuTyf8Hp9NS4R3X1u-3WrSsoKnBIJY4Op7DHYE_pu6Kl3LIXyThIH3R_1MYQj8PUDPrNaDL4oSgAZHCt5gdSyzthHEruydZtje5m1VOWgpN5XRipD_3z8XG1xkLAw8sPABToIGuyrOZngrWBmZFrKMDjqb5fHs8HlCDkBoFpZ5HQewqquzNhGfp9b5j18tQ3IbSdkVTySYgwYKQNzt1GN3UCFyx9PyC7X8wfMMVMnNbdDIACUZLweAZ1ierPUjL0wXsd4QRIKhNwdTiIQMzs2vKL76910HIvWbhC87m0YNvK-P8hgPaoZsg6aajj1MUYIAd5xMU_SAhHe41dxJqijdaFagrkeR6EOJ-RdAv3C6DCTG_9cGicwAOV_fNm8Qk-WqKAfoKe7kDFEAThmrPs3x61H-MgG9KH2NZ5qXo7TDJ9vq5I1Vk3OZYmgYadPKnPoRc_YKSBKEOqbezky64Ws5ns8zVMfGZCZNcwZQVli0OTJMx8FjBGdfGRThTrHVwH3uy2Fwx7kC5ffMV7Kbs6vzzjuVA/F-x371hs";
	char filePath[] = "C:\\Users\\vagrant\\AppData\\Local\\Temp\\nyan_cat.mp3"; // TODO: Change filePath in order to test
	fileIO.DownloadFileViaUrl(url, filePath);
}

int main() {
	// testReadFileContent();
	// testDownloadFileViaUrl();

	return 0;
}
