#define ETHER_ADDR_LEN 6
#define ETHER_HDR_LEN 14

struct ether_hdr {
  unsigned char ether_dest_addr[ETHER_ADDR_LEN]; // 宛先のMACアドレス
  unsigned char ether_src_addr[ETHER_ADDR_LEN];  // 送信元のMACアドレス
  unsigned short ether_type; // イーサネットパケットのタイプ
};


struct ip_hdr {
  unsigned char ip_version_and_header_length; // バージョンとヘッダ長
  unsigned char ip_tos;          // サービスのタイプ
  unsigned short ip_len;         // トータルの長さ
  unsigned short ip_id;          // 識別数値
  unsigned short ip_frag_offset; // フラグメントのオフセットとフラグ
  unsigned char ip_ttl;          // 寿命
  unsigned char ip_type;         // プロトコルタイプ
  unsigned short ip_checksum;    // チェックサム
  unsigned int ip_src_addr;      // 送信元のIPアドレス
  unsigned int ip_dest_addr;     // 宛先のIPアドレス
};


struct tcp_hdr {
  unsigned short tcp_src_port;   // 送信元のTCPポート
  unsigned short tcp_dest_port;  // 宛先のTCPポート
  unsigned int tcp_seq;          // TCPのシーケンス番号
  unsigned int tcp_ack;          // TCPの確認応答番号
  unsigned char reserved:4;      // 6ビットの予約済み領域からの4ビット
  unsigned char tcp_offset:4;    // ホストがリトルエンディアンの場合のTCPデータのオフセット
  unsigned char tcp_flags;       // TCPフラグ（および予約済み領域からの2ビット）
#define TCP_FIN   0x01
#define TCP_SYN   0x02
#define TCP_RST   0x04
#define TCP_PUSH  0x08
#define TCP_ACK   0x10
#define TCP_URG   0x20
  unsigned short tcp_window;     // TCPのウィンドウサイズ
  unsigned short tcp_checksum;   // TCPのチェックサム
  unsigned short tcp_urgent;     // TCPの緊急ポインタ
};


int send_string(int sockfd, unsigned char *buffer) {
   int sent_bytes, bytes_to_send;
   bytes_to_send = strlen(buffer);
   while(bytes_to_send > 0) {
      sent_bytes = send(sockfd, buffer, bytes_to_send, 0);
      if(sent_bytes == -1)
         return 0; // 失敗時には0を返す。
      bytes_to_send -= sent_bytes;
      buffer += sent_bytes;
   }
   return 1; 
}

int recv_line(int sockfd, unsigned char *dest_buffer) {
#define EOL "\r\n" 
#define EOL_SIZE 2
   unsigned char *ptr;
   int eol_matched = 0;

   ptr = dest_buffer;
   while(recv(sockfd, ptr, 1, 0) == 1) { 
      if(*ptr == EOL[eol_matched]) {
         eol_matched++;
         if(eol_matched == EOL_SIZE) { 
            *(ptr+1-EOL_SIZE) = '\0';
            return strlen(dest_buffer); 
         }
      } else {
         eol_matched = 0;
      }
      ptr++;
   }
   return 0;
}
