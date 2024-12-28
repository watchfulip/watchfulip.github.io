const char*AUTHOR="Watchful_IP";
const char*URL="watchfulip.github.io";

const char* VDATE="28-12-24";
const char*VERSION="0.0.2";

/*Feel free to do what you like with this code as long as it is not 
causing harm (please consider including a link to https://watchfulip.github.io/). /*

/*
    For decrypting TP-Link firmware
    
    v0.0.2   added second RSA key and adjusted detection and offsets for that firmware format
    
    Ref:    https://static.tp-link.com/upload/gpl-code/2022/202211/20221130/c310v2_GPL.tar.bz2
    /camera_slp/torchlight/tp_package/lib/libsecurity/src
    
    gcc decrypt_fw.c -o decrypt_fw -L. -lsecurity -Wl,-rpath,.
    
    also valid for:  
    
    Tapo_C225v2_en_1.0.7_Build_231022_Rel.37222n_up_boot-signed_1697944831712.bin
    Tapo_C425v1_en_1.2.10_Build_230817_Rel.66253n_up_boot-signed_1694757161571.bin
    Tapo_C520WSv1_en_1.0.11_Build_230621_Rel.72773n_up_boot-signed_1689911553894.bin
    Tapo_C720v1_en_1.0.15_Build_230309_Rel.45493n_up_boot-signed_1692170680796.bin
    Tapo_D130v1_en_1.0.16_Build_230829_Rel.56497n_up_boot-signed_1696921651739.bin
    D230-up-ver1-1-9-P1[20230614-rel54173]-signed_1686728417801.bin
    Tapo_D235v1_en_1.1.2_Build_231012_Rel.41779n_up_boot-signed_1697200873075.bin.rollback
    Tapo_TC71v2_en_1.3.8_Build_230913_Rel.59332n_up_boot-signed_1695869613098.bin
    H200-up-ver1-2-23-P1\[20231012-rel49638\]-signed_1697774013433.bin 
    
    ax90v1-up-ver1-1-9-P120220512-rel83908_2048_si_1656329037964.bin
    ax11000v2-up-ver2-1-6-P1[20230907-rel65510]_2048_sign_2023-09-07_18.31.59_1694737907340.bin
    axe300v1-up-ver1-0-6-P420230109-rel13284_2048__1677120050594.bin
    be9300v1-up-all-ver1-0-3-P1[20230911-rel70411]_sign_2023-09-11_20.23.49_1695823725657.bin
    XE75 Pro 2.0_en_1.2.6 Build 20230904 Rel. 50316_SG_up_RSA2048_1694166784549.bin
    
    Not valid for:
    
    TL-S* switches
*/


/*	THIS CODE WILL NOT WORK UNTIL REDACTED RSA KEY(S) POPULATED
    
	Check for TP-Link RSA keys taken from their own unencrypted firmware they themselves host
	http://download.tplinkcloud.com/firmware/ax6000v2-up-ver1-1-2-P1[20230731-rel41066]_1024_nosign_2023-07-31_11.26.17_1693471186048.bin.rollback
	http://download.tplinkcloud.com/firmware/Tapo_C210v1_en_1.3.1_Build_221218_Rel.73283n_u_1679534600836.bin	
*/
exit(1);
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define DEBUG



// -libsecurity compiled from https://static.tp-link.com/upload/gpl-code/2022/202211/20221130/c310v2_GPL.tar.bz2
extern int rsaVerifyPSSSignByBase64EncodePublicKeyBlob(unsigned char *pPublicKeyBlob,
    unsigned long PublicKeyBlobLen,
    unsigned char *pInData,
    unsigned long inDataLen,
    unsigned char *PInSignature,
    unsigned long inSignatureLen,
    unsigned int bCheckOnly,
    unsigned int header_align_size,
unsigned int data_dec_offset);  //added data_dec_offset due to different platforms having encrypted data at different offsets - could use #extern in libsecurity instead

#define FW_TYPE_OFFSET 0x14
#define FW_TYPE_STRING "fw-type:"
#define FW_TYPE_LENGTH 8  

int detect_firmware_ver(const unsigned char *fw_buffer, size_t buffer_size) 
{
    
    if (buffer_size < FW_TYPE_OFFSET + FW_TYPE_LENGTH) {
        return -1;  
    }
    
    if (memcmp(fw_buffer + FW_TYPE_OFFSET, FW_TYPE_STRING, FW_TYPE_LENGTH) == 0) {
        printf("%s found\n",FW_TYPE_STRING);
        return 1;  
    }
    
    return 0;  
}

int main(int argc, char *argv[]) 
{
	printf("TP-link firmware decrypt\n%s %s v%s\n%s\n\n", AUTHOR, VDATE, VERSION,URL);
	
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <firmware_file>\n", argv[0]);
        return 1;
    }
    
    const char *firmware_file = argv[1];
    FILE *fp = fopen(firmware_file, "rb");
    if (!fp) {
        perror("Error opening firmware file\n");
        return 1;
    }
    
    fseek(fp, 0, SEEK_END);
    unsigned int fw_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    printf("fw_size is %u\n", fw_size);
    
    // Read firmware into buffer
    unsigned char *fw_buffer = calloc(1,fw_size);
    if (!fw_buffer) {
        perror("Error allocating memory");
        fclose(fp);
        return 1;
    }
    fread(fw_buffer, 1, fw_size, fp);
    fclose(fp);
    //printf("fw_buffer is size %u\n", fw_size);
    
    const char *RSA_KEY; 
    int SIG_SIZE=0;
    int RSA_VER_pos=0;
    int fw_data_pos=0;
    int sig_pos=0;
    int data_dec_offset=0;
    
    
    int firmware_ver = detect_firmware_ver(fw_buffer,fw_size);
    printf("firmware_ver: %d\n",firmware_ver );
    
    switch (firmware_ver) {
        case 1:   //  fw-type: in header
        RSA_KEY="BgIAAAwk <REDACTED> 8e2";	//from ax6000v2-up-ver1-1-2-P1[20230731-rel41066]_1024_nosign_2023-07-31_11.26.17_1693471186048.bin.rollback
        sig_pos=0x130;	
        fw_data_pos=0x14;
        RSA_VER_pos=0x110;
        fw_size-=fw_data_pos;
        data_dec_offset=sig_pos-fw_data_pos;
        break;
        
        case 0:   // Tapo IPC
        RSA_KEY="BgIAAAwk <REDACTED> POz";	 //from Tapo_C210v1_en_1.3.1_Build_221218_Rel.73283n_u_1679534600836.bin
        sig_pos=0x20; 
        RSA_VER_pos=0x00;
        fw_data_pos=0x00;
        data_dec_offset=0;	
        break;
        
        default:
        printf("Unknown firmware_ver %d\n",firmware_ver);
        exit(1);
        break;	     
    }    
    
	uint32_t RSA_VER = (fw_buffer[RSA_VER_pos+0] << 24) | (fw_buffer[RSA_VER_pos+1] << 16) | (fw_buffer[RSA_VER_pos+2] << 8) | fw_buffer[RSA_VER_pos+3];
    
	if (RSA_VER == 0x200) 
	{
		SIG_SIZE = 0x100;
    }
	else if (RSA_VER == 0x100) 
	{
		SIG_SIZE = 0x80;
		printf("RSA-1024 not tested and firmware likely not encrypted\n");
		exit(1);
    }
	else	
	{
		printf("Unable to determine RSA\n");
		exit(1);
    }	
    
	printf("RSA-%d\n", RSA_VER*4);    
    
    // Extract signature
    unsigned char signature[SIG_SIZE];
    memcpy(signature, fw_buffer + sig_pos, sizeof(signature));
    memset(fw_buffer + sig_pos, 0,  sizeof(signature));  //clear sig before verification
    
    //unsigned int header_align_size = ((char)fw_buffer[0x18]<<8 + (char)fw_buffer[0x19]) + 208 + 0x00010000;   //0x00020000 is from Tapo C210 V2 Read Partition function - likely not matter for just decrypt without flash write
    unsigned int header_align_size=0;
    
    unsigned int check_only=0;
    
    #ifdef DEBUG    
        printf("Debug: Calling rsaVerifyPSSSignByBase64EncodePublicKeyBlob with:\n");
        printf("RSA_KEY: %s\n", RSA_KEY);
        printf("RSA_KEY_len: %lu\n", strlen(RSA_KEY));
        printf("fw_buffer address: %p\n", (void*)fw_buffer);
        printf("fw_size: %u\n", fw_size);
        printf("signature address: %p\n", (void*)signature);
        printf("check_only: %d\n", check_only);
        //printf("header_align_size: 0x%08X\n", header_align_size);
        printf("data_dec_offset: 0x%08X\n", data_dec_offset);
        
        
        printf("fw_buffer:\n");
        for (int i = 0; i < 0x180; i++) {
            printf("%02x ", fw_buffer[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");
        
        printf("Signature:\n");
        for (int i = 0; i <  sizeof(signature); i++) {
            printf("%02x ", signature[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");
    #endif
    
	int result = rsaVerifyPSSSignByBase64EncodePublicKeyBlob(
	    (unsigned char *)RSA_KEY,
	    strlen(RSA_KEY),
	    fw_buffer+fw_data_pos,
	    fw_size,
	    signature,
	    256,         //256=pss
	    check_only,
	    0,		//header_align_size likely doesnt matter
	    data_dec_offset);
    
	printf("Debug: rsaVerifyPSSSignByBase64EncodePublicKeyBlob returned: %d\n", result);
    
	if (result !=1) 
	{
		fprintf(stderr, "Firmware verification failed.\n");
		return -1;
	}
	else
		printf("Firmware verification successful\n");
    
    
	char *dec_fw_file = malloc(strlen(firmware_file) + 5);
	if (dec_fw_file == NULL) {
		return -1;
    }
	strcpy(dec_fw_file, firmware_file);
	strcat(dec_fw_file, ".dec");
    
    // Write decrypted firmware to new file
    FILE *out_fp = fopen(dec_fw_file, "wb");
    if (!out_fp) {
        perror("Error creating output file\n");
        free(fw_buffer);
        return 1;
    }
    
    fwrite(fw_buffer, 1, fw_size, out_fp);
    fclose(out_fp);
    
    printf("Decrypted firmware written to %s\n",dec_fw_file);
    
    free(dec_fw_file);
    free(fw_buffer);
    return 0;
}
