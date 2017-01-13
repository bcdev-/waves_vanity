/*      DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
                    Version 2, December 2004 

 Copyright (C) 2016 Eric Kerman <github@bcdev.net> 

 Everyone is permitted to copy and distribute verbatim or modified 
 copies of this license document, and changing it is allowed as long 
 as the name is changed. 

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION 

  0. You just DO WHAT THE FUCK YOU WANT TO.
*/

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<ctype.h>
#include<pthread.h>

#include<time.h>
#include<blake2.h>
#include<libbase58.h>
#include"sha3.h"
#include"sha256.h"
#include<sys/time.h>
#include"curve25519.h"

#include<unistd.h>
#include<sys/statvfs.h>

const int ITERATIONS_PER_LOOP = 513;

typedef struct {
    int threads;
    bool testnet;
    char mask[36];
    char case_mask[36];
} vanity_settings;

void waves_sha3_blake2b_composite(uint8_t *public_key, int data_length, uint8_t *result) {
    blake2b_state S[1];
    sha3_context c;

    blake2b_init(S, 32);
    blake2b_update(S, public_key, data_length);
    blake2b_final(S, result, 32);

    sha3_Init256(&c);
    sha3_Update(&c, result, 32);
    sha3_Finalize(&c);

    memcpy(result, c.sb, 32);
}

void waves_public_key_to_account(uint8_t public_key[32], bool testnet, char *output) {
    char testnet_char = testnet ? 'T' : 'W';
    size_t length = 512;
    uint8_t public_key_hash[32];
    uint8_t without_checksum[512];
    uint8_t checksum[32];
    waves_sha3_blake2b_composite(public_key, 32, (uint8_t*)public_key_hash);

    without_checksum[0] = 0x01;
    without_checksum[1] = testnet_char;
    memcpy(&without_checksum[2], public_key_hash, 20);

    waves_sha3_blake2b_composite(without_checksum, 22, (uint8_t*)checksum);

    memcpy(&without_checksum[22], checksum, 4);

    b58enc(output, &length, without_checksum, 22 + 4);
    output[length] = 0;
}

void seed_to_address(char *key, bool testnet, char *output) {
    char realkey[1024] = {0, 0, 0, 0};
    memcpy(&realkey[4], key, strlen(key));
    uint8_t privkey[32];

    SHA256_CTX ctx;

    waves_sha3_blake2b_composite((uint8_t*)realkey, strlen(key) + 4, privkey);

    sha256_init(&ctx);
    sha256_update(&ctx, privkey, 32);
    sha256_final(&ctx, privkey);

    privkey[0] &= 248;
    privkey[31] &= 127;
    privkey[31] |= 64;

    uint8_t pubkey[32];

    curve25519_donna_basepoint(pubkey, privkey);

    waves_public_key_to_account(pubkey, testnet, output);
}

void unit_test_1() {
#ifndef SKIP_UNIT_TEST
    uint8_t input[] = "A nice, long test to make the day great! :-)";
    uint8_t output[32];
    uint8_t expected[] = {0x5d, 0xf3, 0xcf, 0x20, 0x20, 0x5d, 0x75, 0xe0, 0x9a, 0xe4, 0x6d, 0x13, 0xa8, 0xd9, 0x9a, 0x16, 0x17, 0x4d, 0x71, 0xc8, 0x4f, 0xfc, 0xc0, 0x03, 0x87, 0xfe, 0xc3, 0xd8, 0x1e, 0x39, 0xdc, 0xbe};
    waves_sha3_blake2b_composite(input, strlen((char*)input), (uint8_t*)output);
    if(memcmp(output, expected, 32) != 0) {
        printf("Unit test 1 failed\n");
        exit(-1);
    }
#endif
}

void unit_test_2() {
#ifndef SKIP_UNIT_TEST
    uint8_t input[] = {0xd8, 0x5b, 0x2f, 0x9e, 0x00, 0xde, 0xa8, 0x88, 0x65, 0x55, 0x3b, 0x6f, 0x69, 0xda, 0x53, 0x18, 0xbe, 0x64, 0x4f, 0x4d, 0x39, 0xa9, 0xc4, 0x8e, 0xba, 0xed, 0x71, 0x46, 0xcb, 0x7a, 0xfb, 0x73};
    char output[512];
    char expected[] = "3PAtGGSLnHJ3wuK8jWPvAA487pKamvQHyQw";
    waves_public_key_to_account(input, false, output);
    if(strcmp(output, expected) != 0) {
        printf("Unit test 2 failed\n");
        exit(-1);
    }
#endif
}

void unit_test_3() {
#ifndef SKIP_UNIT_TEST
    uint8_t input[] = {0xdb, 0x3b, 0xe4, 0xbb, 0x58, 0x3e, 0x58, 0xe5, 0x7b, 0xae, 0xb2, 0xa7, 0xad, 0x40, 0x8f, 0x73, 0xb2, 0x04, 0xab, 0x26, 0xd6, 0x4c, 0x73, 0x0e, 0xbb, 0xe1, 0x4d, 0xd0, 0xaf, 0x33, 0xe8, 0x23};
    char output[512];
    char expected[] = "3Mv61qe6egMSjRDZiiuvJDnf3Q1qW9tTZDB";
    waves_public_key_to_account(input, true, output);
    if(strcmp(output, expected) != 0) {
        printf("Unit test 3 failed\n");
        exit(-1);
    }
#endif
}

void unit_test_4() {
#ifndef SKIP_UNIT_TEST
    uint8_t privkey[] = {0x88, 0x72, 0x7a, 0x03, 0x37, 0x7b, 0xfb, 0xa1, 0xb3, 0x65, 0x5c, 0x5e, 0xcb, 0x97, 0x8d, 0xa1, 0x71, 0xe0, 0x24, 0xaa, 0xd7, 0x22, 0xee, 0x49, 0xff, 0xf9, 0x21, 0x4a, 0x74, 0x7e, 0x70, 0x61};
    uint8_t expected[] = {0x92, 0xf2, 0xc1, 0x71, 0xcb, 0x60, 0x78, 0xe6, 0x05, 0x50, 0xcb, 0x99, 0x53, 0xfc, 0x3f, 0x11, 0x80, 0x31, 0xd6, 0x31, 0x4c, 0xb6, 0x40, 0x0d, 0xfd, 0x72, 0x11, 0xf6, 0x01, 0x8d, 0x1d, 0x2b};
    privkey[0] &= 248;
    privkey[31] &= 127;
    privkey[31] |= 64;

    uint8_t output[32];

    static const uint8_t basepoint[32] = {9};
    curve25519_donna(output, privkey, basepoint);
    
    if(memcmp(output, expected, 32) != 0) {
        printf("Unit test 4 failed\n");
        exit(-1);
    }
#endif
}

void unit_test_5() {
#ifndef SKIP_UNIT_TEST
    char test[] = "industry detail rifle scan weird join crawl connect demand top club hello entry second cargo";
    char output[512];
    char expected[] = "3NCyi16BFfFvYhCeg1pKrMKMLDXwazkPuhP";
    seed_to_address(test, true, output);
    if(strcmp(output, expected) != 0) {
        printf("Unit test 5 failed\n");
        exit(-1);
    }
#endif
}

void unit_test_6() {
#ifndef SKIP_UNIT_TEST
    char test[] = "try south announce math salute shoe blast finish state battle nest tube enjoy yellow layer";
    char output[512];
    char expected[] = "3PJXLWbp5ft3LCeesqgJyTpGQRgU9nTY3PA";
    seed_to_address(test, false, output);
    if(strcmp(output, expected) != 0) {
        printf("Unit test 6 failed\n");
        exit(-1);
    }
#endif
}

void unit_test() {
    unit_test_1();
    unit_test_2();
    unit_test_3();
    unit_test_4();
    unit_test_5();
    unit_test_6();
}

void get_entropy(uint8_t entropy[64]) {
    uint8_t entropy_seed[32];

    FILE *fp;
    fp = fopen("/dev/urandom", "r");
    if(fread(&entropy_seed, 1, 32, fp) < 32) {
        printf("Error: Cannot get entropy\n");
        exit(-1);
    }
    fclose(fp);

    struct timeval tval;
    gettimeofday(&tval, NULL);
   
    blake2b_state S[1];
    blake2b_init(S, 64);
    blake2b_update(S, entropy_seed, 256);
 
    for(int i = 0 ; i < 10 ; i++) {
        blake2b_update(S, (char*)&tval, sizeof(struct timeval));
        unit_test_1();
        blake2b_update(S, (char*)&tval, sizeof(struct timeval));
        unit_test_2();
        blake2b_update(S, (char*)&tval, sizeof(struct timeval));
        unit_test_3();
        blake2b_update(S, (char*)&tval, sizeof(struct timeval));
        unit_test_4();
        blake2b_update(S, (char*)&tval, sizeof(struct timeval));
        unit_test_5();
        blake2b_update(S, (char*)&tval, sizeof(struct timeval));
        unit_test_6();
        blake2b_update(S, (char*)&tval, sizeof(struct timeval));
    }
    blake2b_final(S, entropy, 64);
}

bool check_char(vanity_settings *settings, char address[50], int i) {
    char *mask = settings->mask;
    char *case_mask = settings->case_mask;

    if(case_mask[i] == '_') {
        if(mask[i] == '_')
            return true;
        return mask[i] == address[i];
    }
    else if(case_mask[i] == 'n') {
        // Numbers
        return address[i] >= '1' && address[i] <= '9';
    }
    else if(case_mask[i] == 'p') {
        // Uppercase + numbers
        return (address[i] >= '1' && address[i] <= '9') || (address[i] >= 'A' && address[i] <= 'Z');
    }
    else if(case_mask[i] == 'l') {
        // Lowercase
        return address[i] >= 'a' && address[i] <= 'z';
    }
    else if(case_mask[i] == 'u') {
        // Uppercase
        return address[i] >= 'A' && address[i] <= 'Z';
    }
    else if(case_mask[i] == 'x') {
        if(mask[i] == '_')
            return true;
        char alt = mask[i] < 'a'? mask[i] + 32 : mask[i] - 32;
        if(address[i] != mask[i] && address[i] != alt)
            return false;
        return true;
    }
    return false;
}

bool check_mask(vanity_settings *settings, char address[50]) {
    char *mask = settings->mask;

    size_t len = strlen(mask);
    for(int i = 0 ; i < len ; i++)
        if(check_char(settings, address, i) == false)
            return false;
    return true;
}

char base58_map[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

int base58char_to_i(char ch) {
    for(int i = 0 ; i < 58 ; i++) {
        if(base58_map[i] == ch)
            return i;
    }
    return -1;
}

uint64_t heat_map[35][58] = {{0, 0, 14656000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6927467, 7728533, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {632971, 624821, 621092, 623230, 679851, 629526, 629914, 627496, 623598, 625143, 626763, 625468, 158660, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52622, 621867, 624489, 622960, 625518, 621041, 620622, 627099, 630810, 625407, 631001, 624031}, {260981, 258609, 262331, 257055, 263180, 256191, 256940, 259656, 256646, 316365, 259986, 258937, 257342, 260025, 254023, 247215, 247036, 246685, 249763, 249788, 245847, 245014, 247105, 250159, 248834, 249653, 249691, 246771, 250063, 249446, 246423, 245333, 249305, 247159, 249180, 248246, 246870, 243792, 251559, 246160, 248210, 245085, 246093, 249718, 248985, 250750, 248742, 246603, 248974, 247623, 251970, 248116, 247247, 257720, 255692, 259973, 257665, 261470}, {250857, 251962, 254161, 253861, 252609, 250223, 253954, 248251, 252419, 248857, 256193, 248873, 251297, 247355, 252799, 248313, 251711, 252863, 245515, 249625, 253263, 253581, 251900, 255071, 252260, 254276, 255514, 254958, 251533, 245776, 252724, 254554, 249695, 254340, 252953, 255018, 253061, 256248, 250482, 249032, 249133, 306097, 252939, 249576, 249620, 252356, 252189, 250683, 250011, 252769, 252138, 250291, 251597, 252505, 255793, 251944, 250426, 247996}, {250521, 250777, 253290, 247731, 252597, 248927, 308949, 251608, 250282, 249765, 251857, 254769, 248017, 253675, 250150, 250240, 250113, 251006, 253522, 250939, 253767, 253905, 253704, 254681, 253025, 253878, 253611, 252909, 249431, 250081, 251986, 251781, 250715, 251711, 251597, 248223, 252157, 249098, 250396, 251793, 254375, 248979, 253116, 253533, 252147, 253598, 246885, 249391, 254837, 247068, 255063, 255497, 253592, 252370, 251323, 249550, 255417, 252075}, {248268, 249760, 252529, 252459, 248989, 252095, 247989, 252912, 251520, 246457, 249980, 256893, 251515, 251649, 250519, 250315, 253574, 251084, 254371, 255126, 251188, 255997, 249177, 247903, 251007, 252461, 255687, 250437, 249027, 253522, 248945, 252472, 251792, 309126, 251707, 253256, 250720, 252730, 251251, 254106, 252028, 250333, 249327, 249687, 254396, 254541, 251758, 248339, 252475, 252939, 249962, 253769, 252788, 252798, 250667, 253991, 253726, 251961}, {254571, 252384, 252716, 252839, 251623, 253176, 253694, 250354, 250997, 249753, 255189, 248353, 254117, 252951, 253220, 250781, 250810, 250352, 252138, 253118, 252665, 249354, 249538, 251108, 253304, 248284, 250810, 254080, 249634, 252178, 251371, 254878, 248620, 251260, 250777, 251145, 309763, 250303, 253212, 251125, 254769, 253141, 249630, 253143, 254043, 249452, 251193, 251420, 248553, 248318, 254766, 249116, 254307, 248309, 247253, 253128, 253666, 255248}, {247992, 252341, 253503, 250048, 252968, 251698, 252380, 249685, 248681, 252366, 255034, 256856, 250812, 251915, 252292, 250447, 250785, 252441, 256695, 247817, 250066, 248108, 253453, 251970, 255535, 253891, 251603, 249233, 254440, 254264, 251342, 249555, 252217, 251366, 252998, 253698, 250322, 248629, 253578, 254173, 247577, 247808, 250028, 252912, 252301, 251979, 248000, 249702, 308354, 250224, 252267, 252692, 252809, 251512, 254692, 253259, 249930, 252757}, {253238, 256081, 250805, 254602, 309497, 253161, 252174, 249725, 250547, 250792, 253577, 252292, 250452, 252052, 251876, 250469, 247595, 251370, 250471, 250318, 254083, 255230, 250971, 251116, 251900, 246994, 250552, 254557, 248424, 259537, 252309, 249719, 251333, 254974, 250100, 250119, 254263, 251292, 253719, 249830, 250162, 249322, 251998, 250080, 251872, 254604, 248177, 250747, 249534, 256532, 249583, 256655, 251233, 250257, 251311, 246913, 252851, 252053}, {250459, 249490, 251905, 254178, 253059, 250337, 250501, 251153, 251219, 250992, 250105, 247702, 251255, 250228, 253116, 252989, 251590, 251970, 250373, 256741, 253318, 251412, 251935, 249674, 248239, 308767, 255384, 252926, 250905, 251056, 251817, 250812, 252905, 246794, 251260, 255356, 255042, 250050, 254089, 251952, 251811, 254097, 250316, 251522, 255140, 250132, 250756, 252370, 251336, 250882, 253752, 250382, 256929, 248092, 247399, 253251, 250151, 254627}, {252394, 250563, 254450, 252289, 255414, 253937, 253035, 254025, 253795, 251177, 307498, 251917, 249923, 249718, 251541, 251951, 251885, 252055, 256196, 250579, 250102, 252552, 249350, 252660, 252024, 246591, 253508, 246918, 250257, 251369, 251457, 252016, 253126, 252150, 249865, 251440, 252928, 251150, 251292, 252722, 252878, 249571, 248652, 246551, 248964, 255043, 250567, 251689, 248173, 255668, 254875, 254791, 251439, 249763, 255995, 253309, 251079, 249174}, {250866, 252233, 255304, 252414, 250343, 250193, 252563, 253526, 249608, 249692, 254442, 250928, 252311, 252612, 251076, 254543, 249771, 249644, 247363, 253097, 252272, 248667, 250279, 253948, 252185, 249252, 257280, 251630, 250449, 250782, 307567, 255886, 251523, 250765, 253146, 255175, 250338, 249997, 251720, 250779, 251977, 246690, 248783, 252101, 248405, 248351, 256685, 253416, 251887, 254337, 252399, 251729, 249735, 252924, 255903, 252841, 251878, 249790}, {250365, 253386, 251259, 249741, 251776, 252537, 250946, 254807, 251122, 253959, 248353, 251906, 252770, 252073, 248950, 248753, 250752, 251160, 255304, 250021, 257054, 249833, 253062, 247263, 252000, 246637, 247588, 252612, 250848, 249455, 249801, 252529, 250104, 254539, 253475, 252138, 246550, 255744, 253064, 253496, 255881, 250541, 252786, 251247, 250621, 253086, 250619, 253399, 252964, 251432, 247699, 253955, 252576, 312303, 253114, 250834, 251893, 253318}, {250409, 251111, 256790, 254272, 253168, 249636, 250756, 257831, 252371, 251435, 309203, 250892, 251184, 250163, 251188, 253962, 253150, 251171, 251951, 252181, 250448, 247349, 252799, 248684, 252991, 250758, 249153, 248674, 252475, 255529, 253592, 253564, 248286, 256442, 249902, 253993, 255020, 252143, 250438, 252266, 250864, 252552, 252981, 251420, 250274, 249560, 250432, 253561, 249672, 248350, 252023, 251477, 253524, 248612, 253841, 249790, 252157, 247580}, {250178, 254717, 247719, 251880, 255149, 256861, 253044, 250332, 251887, 253885, 248086, 251451, 249436, 249313, 251875, 254232, 251280, 251542, 253142, 250156, 255722, 251861, 252997, 250484, 253145, 254934, 251868, 251742, 250036, 250156, 253121, 311092, 253777, 251442, 249700, 253285, 247398, 249685, 249395, 252249, 253997, 249553, 250736, 252034, 250352, 249490, 252120, 249454, 253430, 253653, 253269, 253515, 252288, 249981, 253165, 252203, 246433, 250073}, {251348, 252675, 252579, 249585, 247839, 251176, 249127, 251436, 251687, 255500, 254766, 252805, 253097, 252101, 250528, 252317, 248640, 253665, 252339, 254338, 313489, 255174, 252876, 250238, 253874, 247587, 252753, 254035, 251308, 254037, 248660, 255085, 252069, 253120, 250942, 252646, 252169, 254489, 253788, 253562, 248602, 248758, 255217, 254826, 249714, 247280, 254343, 245298, 249218, 253903, 251472, 250550, 249631, 250135, 250848, 248363, 247004, 251389}, {253374, 251986, 250870, 249548, 250581, 252525, 250877, 253888, 250603, 251497, 250190, 253974, 250604, 251725, 254157, 252374, 252870, 250058, 249279, 251042, 249967, 253929, 248643, 252487, 251937, 250348, 254118, 251674, 251109, 251234, 251720, 307272, 248142, 255492, 254842, 248432, 249726, 247034, 247327, 251110, 251896, 251560, 252272, 251197, 255290, 251594, 250896, 252860, 250599, 249466, 253025, 252357, 251884, 255555, 254709, 254123, 254353, 253799}, {251864, 247663, 251484, 251334, 306376, 251623, 251470, 254896, 252668, 250441, 250029, 252508, 251355, 251023, 245544, 250764, 253396, 255061, 253002, 252063, 252031, 250900, 248552, 251939, 251317, 248391, 249377, 249433, 253541, 252597, 248748, 252301, 245647, 253988, 250070, 251734, 254798, 254677, 256364, 250654, 254346, 252801, 250697, 251001, 250839, 254918, 252891, 252548, 255496, 248743, 252853, 253436, 255041, 252254, 252498, 251454, 252927, 249634}, {255454, 251729, 249332, 249086, 249708, 248916, 250403, 250963, 249859, 252651, 253268, 253653, 249904, 251619, 251663, 251507, 308629, 249078, 252544, 251703, 247820, 246156, 250998, 252217, 250776, 253857, 256204, 252235, 251966, 252772, 252589, 248952, 249141, 250284, 252261, 254113, 251012, 252603, 254629, 249478, 249731, 251570, 252964, 247747, 255697, 251291, 256488, 250241, 250178, 255043, 255255, 253673, 253020, 251873, 251933, 254399, 251456, 251709}, {256248, 250610, 252919, 251685, 252919, 254563, 252399, 250190, 254939, 252207, 252676, 252788, 251712, 249783, 252219, 250129, 252402, 250073, 246551, 252582, 249889, 249220, 251417, 251338, 310463, 252511, 253841, 254119, 249422, 251232, 250783, 251020, 252128, 253066, 249822, 252668, 254290, 249337, 252724, 255226, 251603, 248796, 249061, 249860, 251849, 250568, 251891, 251753, 248124, 253459, 250994, 251868, 252919, 249338, 251171, 251760, 254081, 252795}, {251057, 252082, 254935, 246649, 308485, 252243, 254181, 254029, 251873, 250586, 246765, 248125, 248460, 252536, 248252, 253443, 253102, 251195, 248461, 247682, 247060, 255481, 250192, 249834, 253241, 252015, 250932, 253511, 251021, 250812, 249562, 250955, 253041, 250658, 253209, 256331, 254843, 254783, 253578, 254924, 251049, 252885, 249813, 247988, 253842, 252274, 252144, 255225, 254171, 253219, 253522, 249505, 252235, 251801, 251634, 250631, 252704, 251234}, {249031, 254835, 248491, 252853, 250806, 252030, 250413, 252188, 253435, 250651, 251717, 252397, 252924, 249977, 250922, 253910, 251951, 248216, 252850, 255883, 251028, 251436, 255030, 253097, 253188, 250802, 251971, 254134, 249078, 250180, 253781, 250562, 305057, 252468, 253367, 253596, 250723, 246910, 253329, 252174, 252189, 251428, 252785, 252797, 248604, 249970, 250545, 253562, 250632, 248766, 253759, 250782, 256228, 253457, 251392, 250669, 251822, 249222}, {255239, 251710, 253103, 255581, 248769, 251098, 248603, 253528, 254268, 251809, 248921, 251455, 251544, 307386, 250107, 251859, 252022, 250318, 249981, 254754, 250821, 249874, 249534, 250704, 249279, 254901, 248094, 254501, 250695, 250118, 247820, 252446, 250954, 252805, 249493, 254892, 253883, 251670, 252851, 249421, 249614, 250514, 257066, 252108, 251240, 253536, 255211, 251098, 249023, 252332, 254384, 252259, 251502, 250594, 252300, 253706, 251047, 251655}, {252596, 251133, 255766, 249226, 248819, 253364, 249361, 250514, 248900, 249001, 250749, 250741, 255396, 253616, 250835, 250710, 252716, 251710, 250323, 248266, 252352, 251916, 253327, 247423, 252863, 253557, 253761, 250612, 250581, 311826, 253637, 256170, 253229, 251703, 249559, 250106, 252640, 251525, 250353, 249145, 252597, 254471, 250132, 249451, 253265, 252415, 252133, 252044, 253499, 254915, 248743, 253196, 249642, 249981, 251634, 255228, 250717, 251910}, {248781, 250830, 251041, 251023, 249334, 251668, 252354, 249692, 252835, 248100, 253569, 252671, 251641, 251196, 251767, 250143, 249114, 256116, 251004, 254209, 254855, 250264, 252017, 250517, 250213, 250100, 251090, 253891, 249642, 251570, 253763, 250286, 252649, 253608, 256366, 253146, 251871, 250252, 252902, 248711, 251509, 250325, 254212, 253746, 252490, 253773, 252542, 248241, 253984, 252670, 251380, 251363, 249636, 248350, 249677, 309123, 253298, 254880}, {256327, 247990, 248620, 251260, 250721, 254280, 255249, 251662, 253739, 253489, 248501, 253557, 251306, 252320, 253512, 249645, 251091, 255600, 249664, 251019, 245958, 253708, 251020, 250538, 250227, 253785, 311010, 249665, 251932, 251438, 251331, 250626, 249933, 253543, 252107, 251579, 251657, 249619, 249679, 251759, 255423, 250384, 253273, 249796, 250331, 252215, 252060, 252202, 251543, 252254, 247766, 253724, 250283, 255901, 254631, 250514, 251308, 251726}, {248843, 256138, 253948, 250893, 247860, 255588, 247773, 252540, 251761, 254311, 249612, 255056, 249367, 253995, 250960, 253976, 255812, 248951, 251982, 250697, 251600, 250215, 252615, 252132, 249144, 249201, 249829, 250258, 253054, 248275, 248058, 250709, 250988, 251822, 251138, 252093, 254370, 250208, 251057, 252178, 249902, 253278, 248079, 253958, 251832, 254008, 254396, 250450, 253643, 248847, 255702, 251719, 250024, 257595, 251500, 251557, 250754, 309749}, {251124, 253473, 306423, 249473, 256873, 251405, 252633, 251526, 249256, 250689, 254138, 252890, 252825, 248979, 251271, 250945, 253760, 250497, 249982, 247877, 251701, 250029, 253583, 250865, 248823, 251123, 250013, 257271, 248054, 251127, 257651, 251974, 251795, 252629, 255338, 253682, 249631, 253827, 254685, 251838, 251712, 252021, 246658, 250816, 250546, 251810, 250248, 251252, 249773, 253140, 246596, 250635, 252805, 253064, 252603, 252504, 252904, 255235}, {248754, 248661, 255505, 251618, 250257, 252590, 246825, 248739, 254287, 249941, 253290, 254426, 253208, 253506, 254735, 253803, 249717, 251988, 309943, 255186, 253058, 250792, 252444, 253793, 249375, 249832, 256605, 251949, 253874, 251519, 251846, 255988, 253600, 249605, 248232, 248837, 250983, 250460, 249772, 251640, 249168, 251297, 249343, 249459, 251016, 253017, 251393, 252002, 250947, 252599, 253903, 252544, 254568, 252342, 250322, 253161, 249570, 248166}, {251817, 250871, 249260, 251028, 249826, 248612, 251848, 255450, 250421, 253794, 250969, 250611, 251366, 255459, 254454, 254185, 251788, 303790, 252318, 250076, 250848, 251065, 251282, 252331, 253147, 249706, 251248, 248455, 251587, 252076, 251464, 248345, 250061, 256527, 254310, 251136, 248948, 250538, 252201, 255480, 250772, 252835, 251013, 252204, 252215, 252683, 250907, 251644, 249201, 253331, 251477, 252755, 253339, 256604, 249570, 252314, 251758, 252680}, {249373, 251247, 251948, 250588, 250007, 250593, 249430, 253904, 251401, 250671, 252154, 251007, 253406, 245974, 247869, 252107, 248567, 255313, 249944, 256382, 251165, 258111, 252249, 310882, 251829, 250580, 250465, 251345, 248095, 248406, 247265, 253078, 255273, 255425, 253581, 249426, 254877, 249593, 252212, 251134, 251016, 253453, 252500, 251355, 248623, 252486, 255532, 255465, 250331, 250793, 249393, 251597, 252178, 250122, 250876, 260018, 253934, 249452}, {251215, 254998, 251102, 252084, 253334, 254638, 251057, 251224, 248425, 250872, 251175, 255019, 253969, 256075, 251853, 250997, 249872, 249516, 254002, 250135, 247145, 253492, 253511, 252626, 252918, 248677, 254083, 249371, 255489, 252209, 247266, 253372, 248370, 252846, 309407, 254046, 250220, 250058, 247856, 252102, 253152, 253776, 253179, 250348, 250190, 255135, 251752, 254510, 251477, 247677, 250089, 252044, 249404, 252085, 249248, 248265, 250225, 256818}, {249295, 253365, 253694, 253024, 250064, 252213, 255635, 253023, 247733, 254847, 251308, 251931, 251870, 250537, 254335, 252320, 247046, 254481, 251871, 250464, 252837, 251242, 250060, 251337, 248232, 247490, 250477, 252349, 249258, 253019, 247696, 254124, 255347, 248655, 253881, 254496, 254475, 252167, 256282, 254060, 253602, 250489, 251795, 251987, 253190, 253859, 243808, 249709, 251412, 255572, 253518, 254687, 251685, 250282, 250270, 249406, 305746, 248443}, {251744, 251749, 249639, 254101, 253229, 249956, 255614, 254752, 251634, 254132, 248778, 251912, 309333, 253524, 250288, 250431, 247885, 253170, 250063, 247997, 252002, 254487, 248411, 251487, 250076, 252877, 250231, 247828, 251544, 254371, 247619, 250172, 251438, 250924, 249389, 246581, 251590, 254088, 251102, 252030, 250021, 254347, 254676, 253819, 253749, 251890, 252931, 253890, 252728, 252327, 250647, 251488, 249989, 254423, 249658, 253168, 254973, 253098}};
double heat_map_f[35][58];

void calculate_heat_map() {
    for(int row = 0 ; row < 35 ; row++) {
        uint64_t heat_map_sum = 0;
        for(int i = 0 ; i < 58 ; i++)
            heat_map_sum += heat_map[row][i];
        for(int i = 0 ; i < 58 ; i++)
            heat_map_f[row][i] = 1. * heat_map[row][i] / heat_map_sum;
    }
}

inline
void fakebase58(char seed[128], uint8_t entropy[64]) {
    uint16_t *ent = (uint16_t*)entropy;
    for(int i = 0 ; i < 27 ; i++) {
        seed[i] = base58_map[ent[i] % 58];
    }
    seed[27] = 0;
}

int generate_addresses(bool testnet, int iterations, vanity_settings *settings, char seed[128], char address[50], uint8_t entropy[64]) {
    blake2b_state S[1];
    blake2b_init(S, 64);
    blake2b_update(S, entropy, 64);
    blake2b_final(S, entropy, 64);

    uint32_t *ent = (uint32_t*)entropy;
    for(int i = 0; i < iterations ; i++) {
        ent[0]++;
        if(i % 58 == 0)
            ent[1]++;

        fakebase58(seed, entropy);

        seed_to_address(seed, testnet, address);

//        for(int u = 0 ; u < strlen(address) ; u++)
//           heat_map[u][base58char_to_i(address[u])]++;

//        fprintf(stderr, "Seed: %s\n", seed);
//        fprintf(stderr, "Address: %s\n", address);

        if(check_mask(settings, address))
            return i;
    }
    return iterations;
}

void print_heat_map() {
    printf("\n\n{");
    for(int row = 0 ; row < 35 ; row++) {
        printf("{");
        for(int i = 0 ; i < 58 ; i++) {
            printf("%lu", heat_map[row][i]);
            if(i < 57)
                printf(", ");
        }
        printf("}");
        if(row < 34)
            printf(", ");
    }
    printf("}\n\n");
}

void print_heat_map_f() {
    printf("\n\n");
    for(int row = 0 ; row < 35 ; row++) {
        for(int i = 0 ; i < 58 ; i++) {
            printf("%f", heat_map_f[row][i]);
            if(i < 57)
                printf(", ");
        }
        printf("\n");
    }
    printf("\n\n");
}

#define UINT64_T_MAX 18446744073709551615UL

double probability_of_char(vanity_settings *settings, int i) {
    char *mask = settings->mask;
    char *case_mask = settings->case_mask;

    double probability = 0;
    if(case_mask[i] == '_') {
        if(mask[i] == '_')
            return 1;
        if(base58char_to_i(mask[i]) < 0)
            return 0;

        return heat_map_f[i][base58char_to_i(mask[i])];
    }
    else if(case_mask[i] == 'p') {
        // Numbers
        for(int u = 0 ; u < 9 ; u++)
            probability += heat_map_f[i][u];
        for(int u = 9 ; u < 9 + 24 ; u++)
            probability += heat_map_f[i][u];
        return probability;
    }
    else if(case_mask[i] == 'n') {
        // Numbers
        for(int u = 0 ; u < 9 ; u++)
            probability += heat_map_f[i][u];
        return probability;
    }
    else if(case_mask[i] == 'l') {
        // Lowercase
        for(int u = 9 + 24 ; u < 9 + 24 + 25 ; u++)
            probability += heat_map_f[i][u];
        return probability;
    }
    else if(case_mask[i] == 'u') {
        // Uppercase
        for(int u = 9 ; u < 9 + 24 ; u++)
            probability += heat_map_f[i][u];
        return probability;
    }
    else if(case_mask[i] == 'x') {
        if(mask[i] == '_')
            return 1;
        char alt = mask[i] < 'a'? mask[i] + 32 : mask[i] - 32;
        double prob = 0;
        if(base58char_to_i(mask[i]) > 0)
            prob += heat_map_f[i][base58char_to_i(mask[i])];
        if(base58char_to_i(alt) > 0)
            prob += heat_map_f[i][base58char_to_i(alt)];
        return prob;
    }
    return 0;
}

uint64_t calculate_probability_50(vanity_settings *settings) {
    double probability = 1.;
    for(int i = 0 ; i < 35 ; i++)
        probability *= probability_of_char(settings, i);
    if (probability == 0.)
        return UINT64_T_MAX;
    return 1 / probability / 2;
}

typedef struct {
    int thread_no;
    vanity_settings *settings;
    uint64_t iterations;
    bool completed;
    char seed[128];
    char address[128];
    bool keep_working;
} worker_thread_struct;

void display_settings(vanity_settings settings) {
    printf("Vanity miner settings:\n");
    printf("CPU threads: %d\n", settings.threads);
    printf("Network: ");
    if(settings.testnet)
        printf("Testnet\n");
    else
        printf("Mainnet\n");
    printf("Char mask: %s\n", settings.mask);
    printf("Case mask: %s\n\n", settings.case_mask);
}

vanity_settings default_settings() {
    vanity_settings settings;
    settings.threads = sysconf(_SC_NPROCESSORS_ONLN);
    settings.testnet = false;
    strcpy(settings.mask, "___________________________________");
    strcpy(settings.case_mask, "___________________________________");

    return settings;
}

void help(int argc, char **argv) {
    printf("Usage:\n  %s [OPTION...]\n\n", argv[0]);
    printf("Help Options:\n");
    printf("  -h                Show help options\n\n");
    printf("Waves Options:\n");
    printf("  -n {t or m}         t - Testnet, m - Mainnet [default]\n\n");
    printf("Mask Options:\n");
    printf("  -m {mask}         Char mask. _ is 'any character at this position'.\n  Any other character means 'this specific character at this position'.\n");
    printf("Example: '%s -m ____eaaa' may generate address 3MvMeaaaLm32f5JzsQQxYhqKL2fbrEQStCs.\n\n", argv[0]);
    printf("  -c {mask}         Case mask. ? means 'any case for a character'\n  _ means 'this specific case for a character'\n");
    printf("The default case mask is: ___________________________________\n");
    printf("Case mask can consist only of _, u, l, n, x and p characters.\n");
    printf("They mean:\n");
    printf("  n - any number\n");
    printf("  x - a character from character mask of any case [ex. a or A, c or C]\n");
    printf("  u - any uppercase character\n");
    printf("  l - any lowercase character\n");
    printf("  p - any uppercase character or a number\n");
    printf("  _ - exactly the same character as in character mask.\n\n");
    printf("Example: '%s -m _____________________________N____N -c _____________________________xnnnnx'\n", argv[0]);
    printf("         A sample result: 3PN7C7rasDZr4C48SWeiQbHjPmM4xN2892n\n");
    printf("Example: '%s -c ppppppppppppppppppppppppppppppppppp -n t'\n", argv[0]);
    printf("         A sample result: 3NCL45V25RUUXQ7YKB41G3ACW3KTUFPVYUV\n");
}

void *worker_thread(void *data) {
    worker_thread_struct *worker = (worker_thread_struct*)data;
    vanity_settings *settings = worker->settings;

    uint8_t entropy[64];
    get_entropy(entropy);

    while(worker->keep_working) {
        uint64_t iter = generate_addresses(settings->testnet, ITERATIONS_PER_LOOP, settings, worker->seed, worker->address, entropy);
        worker->iterations += iter;
        if(iter != ITERATIONS_PER_LOOP) {
            worker->iterations++;
            worker->completed = true;
            worker->keep_working = false;
        }
    }
    pthread_exit((void*) data);
}

int address_found(vanity_settings settings, worker_thread_struct *workers, char **seed, char **address) {
    for(int i = 0 ; i < settings.threads ; i++) {
        if(workers[i].completed) {
            *seed = workers[i].seed;
            *address = workers[i].address;
            return i;
        }
    }
    return -1;
}

int sum_iterations(vanity_settings settings, worker_thread_struct *workers) {
    int sum = 0;
    for(int i = 0 ; i < settings.threads ; i++)
        sum += workers[i].iterations;
    return sum;
}

void validate_case_mask(char *case_mask) {
    bool valid = true;

    for(int i = 0 ; i < strlen(case_mask) ; i++)
        if(!(case_mask[i] == '_' || case_mask[i] == 'x' || case_mask[i] == 'n' || case_mask[i] == 'u' || case_mask[i] == 'l' || case_mask[i] == 'p'))
            valid = false;

    if(strlen(optarg) > 35) {
        printf("The case mask cannot be longer than 35 characters.\n");
        exit(1);
    }

    if(valid == false) {
        printf("Case mask can consist only of _, u, l, n, x and p characters.\n");
        printf("They mean:\n");
        printf("  n - any number\n");
        printf("  x - a character from character mask of any case [ex. a or A, c or C]\n");
        printf("  u - any uppercase character\n");
        printf("  l - any lowercase character\n");
        printf("  p - any uppercase character or a number\n");
        printf("  _ - exactly the same character as in character mask.\n");
        exit(1);
    }
}

vanity_settings parse_settings(int argc, char **argv) {
    vanity_settings settings = default_settings();
    int c;
    while ((c = getopt (argc, argv, "hm:t:c:n:")) != -1)
        switch (c)
        {
          case 'h':
            help(argc, argv);
            exit(0);
          case 'm':
            if(strlen(optarg) > 35) {
                help(argc, argv);
                exit(1);
            }
            memcpy(settings.mask, optarg, strlen(optarg));
            break;
          case 'c':
            validate_case_mask(optarg);
            memcpy(settings.case_mask, optarg, strlen(optarg));
            break;
          case 'n':
            if(strlen(optarg) > 1 || !(optarg[0] == 't' || optarg[0] == 'm')) {
                help(argc, argv);
                exit(1);
            }
            if(optarg[0] == 't')
                settings.testnet = true;
            else if(optarg[0] == 'm')
                settings.testnet = false;
            break;
          case 't':
            settings.threads = atoi(optarg);
            break;

          case '?':
            if (isprint (optopt))
                printf("Unknown option -%c.\n\n", optopt);

            help(argc, argv);
            exit(1);
          default:
            abort();
        }

    if(argc > optind) {
        help(argc, argv);
        exit(1);
    }
    return settings;
}

int main(int argc, char **argv) {
    unit_test();

    vanity_settings settings = parse_settings(argc, argv);

    display_settings(settings);

    char *seed, *address;

    calculate_heat_map();
    uint64_t probability_50 = calculate_probability_50(&settings);
    if(probability_50 == UINT64_T_MAX) {
        printf("It's impossible to generate this address.\n");
        return -1;
    }

    worker_thread_struct workers[settings.threads];
    pthread_attr_t attr;
    pthread_t threads[settings.threads];
    for(int i = 0 ; i < settings.threads ; i++) {
        workers[i].thread_no = i;
        workers[i].completed = 0;
        workers[i].iterations = 0;
        workers[i].settings = &settings;
        workers[i].keep_working = true;
    }
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    printf("Iterations expected: %lu\n", probability_50);

    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);

    printf("Starting workers...\n");

    for(int i = 0 ; i < settings.threads ; i++) {
       int rc = pthread_create(&threads[i], &attr, worker_thread, (void *)&workers[i]);
       if(rc) {
          printf("ERROR; return code from pthread_create() is %d\n", rc);
          exit(-1);
       }        
    }

    while(true) {
        usleep(1000 * 500); // 500ms
        uint64_t iterations = sum_iterations(settings, workers);
        int winning_worker = address_found(settings, workers, &seed, &address);
        if(winning_worker < 0) {
            gettimeofday(&tval_after, NULL);
            timersub(&tval_after, &tval_before, &tval_result);
            uint64_t usec = (long int)(tval_result.tv_sec * 1000) + (long int)(tval_result.tv_usec / 1000);//printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
            double speed = iterations;
            speed /= usec;
            speed *= 1000;
            
            uint64_t probability_50_h = (probability_50 / speed) / 3600;
            int probability_50_m = (uint64_t)((probability_50 / speed) / 60) % 60;
            int probability_50_s = (uint64_t)(probability_50 / speed) % 60;

            uint64_t d = (int)tval_result.tv_sec / 3600 / 24;
            int h = ((int)tval_result.tv_sec / 3600) % 24;
            int m = ((int)tval_result.tv_sec / 60) % 60;
            int s = (int)tval_result.tv_sec % 60;

            printf("\r%lu  %lud %dh %dm %ds  %.2f keys/second  50%% chance: %lud %luh %dm %ds", iterations, d, h, m, s, speed, probability_50_h / 24, probability_50_h % 24, probability_50_m, probability_50_s);

            uint64_t probability_95_h = (probability_50 * 4 / speed) / 3600;
            int probability_95_m = (uint64_t)((probability_50 * 4 / speed) / 60) % 60;
            int probability_95_s = (uint64_t)(probability_50 * 4 / speed) % 60;

            printf("  95%% chance: %lud %luh %dm %ds", probability_95_h / 24, probability_95_h % 24, probability_95_m, probability_95_s);
            printf("          \r");
//            print_heat_map();
//            print_heat_map_f();
            fflush(stdout);
        } else {
            printf("\nOverall iterations: %lu\nAddress: %s\nPassword: %s\n", iterations, address, seed);
            break;
        }
    }
    return 0;
}

