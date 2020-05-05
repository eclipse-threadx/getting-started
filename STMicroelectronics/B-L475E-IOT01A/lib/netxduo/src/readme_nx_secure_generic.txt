Revision History

08/15/2019  NX Secure generic code version 5.12. This release includes the
            following major features:

        Files are modified to store cipher suite pointer in the session:
            nx_secure_dtls_hash_record.c
            nx_secure_dtls_process_clienthello.c
            nx_secure_dtls_send_serverhello.c
            nx_secure_dtls_server_handshake.c
            nx_secure_dtls_verify_mac.c
            nx_secure_tls.h
            nx_secure_tls_finished_hash_generate.c
            nx_secure_tls_generate_keys.c
            nx_secure_tls_generate_premaster_secret.c
            nx_secure_tls_hash_record.c
            nx_secure_tls_process_certificate_verify.c
            nx_secure_tls_process_client_key_exchange.c
            nx_secure_tls_process_clienthello.c
            nx_secure_tls_process_server_key_exchange.c
            nx_secure_tls_process_serverhello.c
            nx_secure_tls_process_serverhello_extensions.c
            nx_secure_tls_record_hash_calculate.c
            nx_secure_tls_record_hash_initialize.c
            nx_secure_tls_record_hash_update.c
            nx_secure_tls_record_payload_decrypt.c
            nx_secure_tls_record_payload_encrypt.c
            nx_secure_tls_send_certificate_verify.c
            nx_secure_tls_send_client_key_exchange.c
            nx_secure_tls_send_record.c
            nx_secure_tls_send_server_key_exchange.c
            nx_secure_tls_send_serverhello.c
            nx_secure_tls_server_handshake.c
            nx_secure_tls_session_iv_size_get.c
            nx_secure_tls_session_keys_set.c
            nx_secure_tls_session_reset.c
            nx_secure_tls_verify_mac.c

        Files are modified to optimize RAM usage for remote certificates by
        utilizing otherwise unused space in the TLS packet reassembly buffer
        and parsing the certificates directly within the reassembly buffer:
            nx_secure_tls.h	
            nx_secure_tls_process_record.c	
            nx_secure_tls_process_remote_certificate.c	
            nx_secure_tls_remote_certificate_allocate.c	
            nx_secure_tls_remote_certificate_free_all.c	
            nx_secure_tls_session_packet_buffer_set.c
            nx_secure_tls_session_receive_records.c	
            nx_secure_tls_session_x509_client_verify_configure.c	
            nx_secure_x509.h	
            nxe_secure_tls_session_x509_client_verify_configure.c	

        Files are modified to add TLS session intialization checking:
            nx_secure_tls.h
            nx_secure_tls_session_create.c
            nx_secure_tls_session_delete.c
            nx_secure_tls_session_reset.c
            nxe_secure_dtls_client_protocol_version_override.c
            nxe_secure_dtls_client_session_start.c
            nxe_secure_dtls_packet_allocate.c
            nxe_secure_dtls_psk_add.c
            nxe_secure_dtls_server_session_send.c
            nxe_secure_dtls_server_session_start.c
            nxe_secure_dtls_session_client_info_get.c
            nxe_secure_dtls_session_end.c
            nxe_secure_dtls_session_local_certificate_add.c
            nxe_secure_dtls_session_local_certificate_remove.c
            nxe_secure_dtls_session_receive.c
            nxe_secure_dtls_session_reset.c
            nxe_secure_dtls_session_send.c
            nxe_secure_dtls_session_start.c
            nxe_secure_dtls_session_trusted_certificate_add.c
            nxe_secure_dtls_session_trusted_certificate_remove.c
            nxe_secure_tls_active_certificate_set.c
            nxe_secure_tls_client_psk_set.c
            nxe_secure_tls_local_certificate_add.c
            nxe_secure_tls_local_certificate_find.c
            nxe_secure_tls_local_certificate_remove.c
            nxe_secure_tls_packet_allocate.c
            nxe_secure_tls_psk_add.c
            nxe_secure_tls_remote_certificate_allocate.c
            nxe_secure_tls_remote_certificate_buffer_allocate.c
            nxe_secure_tls_remote_certificate_free_all.c
            nxe_secure_tls_server_certificate_add.c
            nxe_secure_tls_server_certificate_find.c
            nxe_secure_tls_server_certificate_remove.c
            nxe_secure_tls_session_alert_value_get.c
            nxe_secure_tls_session_certificate_callback_set.c
            nxe_secure_tls_session_client_callback_set.c
            nxe_secure_tls_session_client_verify_disable.c
            nxe_secure_tls_session_client_verify_enable.c
            nxe_secure_tls_session_end.c
            nxe_secure_tls_session_packet_buffer_set.c
            nxe_secure_tls_session_protocol_version_override.c
            nxe_secure_tls_session_receive.c
            nxe_secure_tls_session_renegotiate.c
            nxe_secure_tls_session_renegotiate_callback_set.c
            nxe_secure_tls_session_reset.c
            nxe_secure_tls_session_send.c
            nxe_secure_tls_session_server_callback_set.c
            nxe_secure_tls_session_sni_extension_parse.c
            nxe_secure_tls_session_sni_extension_set.c
            nxe_secure_tls_session_start.c
            nxe_secure_tls_session_time_function_set.c
            nxe_secure_tls_session_x509_client_verify_configure.c
            nxe_secure_tls_trusted_certificate_add.c
            nxe_secure_tls_trusted_certificate_remove.c


        Files are modified to fix DTLS version handling:
            nx_secure_dtls_send_clienthello.c
            nx_secure_dtls_send_helloverifyrequest.c
            nx_secure_dtls_server_handshake.c
            nx_secure_dtls_session_start.c

        Files are modified to improve informational error return codes for 
        easier debugging of TLS errors:
            nx_secure_dtls_client_handshake.c    
            nx_secure_dtls_process_clienthello.c    
            nx_secure_dtls_send_clienthello.c    
            nx_secure_dtls_send_serverhello.c    
            nx_secure_dtls_server_handshake.c    
            nx_secure_tls.h    
            nx_secure_tls_map_error_to_alert.c    
            nx_secure_tls_process_client_key_exchange.c    
            nx_secure_tls_process_finished.c    
            nx_secure_tls_process_server_key_exchange.c    
            nx_secure_tls_process_serverhello_extensions.c    
            nx_secure_tls_server_handshake.c    
            nx_secure_tls_ecc_generate_premaster_secret.c    
            nx_secure_tls_ecc_process_certificate_verify.c    
            nx_secure_tls_ecc_process_client_key_exchange.c    
            nx_secure_tls_ecc_process_server_key_exchange.c    
            nx_secure_tls_ecc_send_certificate_verify.c    
            nx_secure_tls_ecc_send_server_key_exchange.c    

        Files are added or modified for the new API 
        "nx_secure_tls_session_alert_value_get":
            nx_secure_dtls_process_record.c
            nx_secure_tls.h    
            nx_secure_tls_api.h    
            nx_secure_tls_session_alert_value_get.c    
            nx_secure_tls_session_start.c
            nxe_secure_tls_session_alert_value_get.c
     
        Files are modified to provide consistent error checking of return
        values from generic NX_CRYPTO cipher routine APIs:
            nx_secure_dtls_hash_record.c
            nx_secure_tls_client_handshake.c
            nx_secure_tls_finished_hash_generate.c
            nx_secure_tls_generate_keys.c
            nx_secure_tls_handshake_hash_init.c
            nx_secure_tls_hash_record.c
            nx_secure_tls_process_certificate_verify.c
            nx_secure_tls_process_client_key_exchange.c
            nx_secure_tls_record_payload_decrypt.c
            nx_secure_tls_send_certificate_verify.c
            nx_secure_tls_send_client_key_exchange.c
            nx_secure_tls_server_handshake.c
            nx_secure_tls_session_keys_set.c
            nx_secure_tls_session_reset.c
            nx_secure_x509_certificate_verify.c
            nx_secure_x509_crl_verify.c
            
        Files are modified to improve the error return code when the
        TLS crypto key buffer is too small for the chosen ciphersuite:
            nx_secure_tls.
            nx_secure_tls_session_keys_set.c

        Files are modified to improve packet length verification:
            nx_secure_dtls_send_clienthello.c
            nx_secure_dtls_send_serverhello.c
            nx_secure_tls_ecc_generate_keys.c
            nx_secure_tls_ecc_send_client_key_exchange.c
            nx_secure_tls_generate_keys.c
            nx_secure_tls_send_clienthello.c
            nx_secure_tls_send_serverhello.c

        Files are modified to add caller checking:
            nxe_secure_tls_active_certificate_set.c
            nxe_secure_tls_client_psk_set.c
            nxe_secure_tls_local_certificate_add.c
            nxe_secure_tls_local_certificate_find.c
            nxe_secure_tls_local_certificate_remove.c
            nxe_secure_tls_metadata_size_calculate.c
            nxe_secure_tls_packet_allocate.c
            nxe_secure_tls_psk_add.c
            nxe_secure_tls_remote_certificate_allocate.c
            nxe_secure_tls_remote_certificate_buffer_allocate.c
            nxe_secure_tls_remote_certificate_free_all.c
            nxe_secure_tls_server_certificate_add.c
            nxe_secure_tls_server_certificate_find.c
            nxe_secure_tls_server_certificate_remove.c
            nxe_secure_tls_session_certificate_callback_set.c
            nxe_secure_tls_session_client_callback_set.c
            nxe_secure_tls_session_client_verify_disable.c
            nxe_secure_tls_session_client_verify_enable.c
            nxe_secure_tls_session_create.c
            nxe_secure_tls_session_delete.c
            nxe_secure_tls_session_end.c
            nxe_secure_tls_session_packet_buffer_set.c
            nxe_secure_tls_session_protocol_version_override.c
            nxe_secure_tls_session_receive.c
            nxe_secure_tls_session_renegotiate.c
            nxe_secure_tls_session_renegotiate_callback_set.c
            nxe_secure_tls_session_reset.c
            nxe_secure_tls_session_send.c
            nxe_secure_tls_session_server_callback_set.c
            nxe_secure_tls_session_sni_extension_parse.c
            nxe_secure_tls_session_sni_extension_set.c
            nxe_secure_tls_session_start.c
            nxe_secure_tls_session_time_function_set.c
            nxe_secure_tls_session_x509_client_verify_configure.c
            nxe_secure_tls_trusted_certificate_add.c
            nxe_secure_tls_trusted_certificate_remove.c
            nxe_secure_x509_certificate_initialize.c
            nxe_secure_x509_common_name_dns_check.c
            nxe_secure_x509_crl_revocation_check.c
            nxe_secure_x509_dns_name_initialize.c
            nxe_secure_x509_extended_key_usage_extension_parse.c
            nxe_secure_x509_extension_find.c
            nxe_secure_x509_key_usage_extension_parse.c

        Files are modified to fix static anaylsis reports:
            nx_secure_tls_generate_keys.c
            nx_secure_tls_process_remote_certificate.c
            nx_secure_tls_session_packet_buffer_set.c
            nx_secure_tls_ecc_generate_keys.c
            nx_secure_tls_ecc_send_server_key_exchange.c

        Files are modified to correct self-signed certificate verification
        and support duplicate certificates sent by a remote host:
            nx_secure_tls_process_remote_certificate.c
            nx_secure_x509_certificate_chain_verify.c

        Files are modified to provide RFC 7507 version fallback SCSV support:
            nx_secure_tls_process_clienthello.c
            nx_secure_dtls_process_clienthello.c
            nx_secure_dtls_send_clienthello.c
            nx_secure_tls.h
            nx_secure_tls_map_error_to_alert.c
            nx_secure_tls_newest_supported_version.c
            nx_secure_tls_protocol_version_get.c
            nx_secure_tls_send_clienthello.c
            nx_secure_tls_send_serverhello_extensions.c
            nx_secure_tls_session_reset.c            

        Files are modified to improve packet length verification:
            nx_secure_dtls_allocate_handshake_packet.c
            nx_secure_dtls_packet_allocate.c
            nx_secure_dtls_process_record.c
            nx_secure_dtls_process_receive_callback.c
            nx_secure_dtls_send_helloverifyrequest.c
            nx_secure_dtls_send_record.c
            nx_secure_dtls_send_serverhello.c
            nx_secure_dtls_session_end.c
            nx_secure_dtls_session_receive.c
            nx_secure_dtls_session_start.c
            nx_secure_tls.h
            nx_secure_tls_allocate_handshake_packet.c
            nx_secure_tls_client_psk_set.c
            nx_secure_tls_generate_keys.c
            nx_secure_tls_generate_premaster_secret.c
            nx_secure_tls_packet_allocate.c
            nx_secure_tls_process_clienthello_extensions.c
            nx_secure_tls_process_serverhello.c
            nx_secure_tls_record_payload_decrypt.c
            nx_secure_tls_send_certificate.c
            nx_secure_tls_send_certificate_request.c
            nx_secure_tls_send_certificate_verify.c
            nx_secure_tls_send_changecipherspec.c
            nx_secure_tls_send_client_key_exchange.c
            nx_secure_tls_send_clienthello.c
            nx_secure_tls_send_clienthello_extensions.c
            nx_secure_tls_send_finished.c
            nx_secure_tls_send_record.c
            nx_secure_tls_send_server_key_exchange.c
            nx_secure_tls_send_serverhello.c
            nx_secure_tls_send_serverhello_extensions.c
            nx_secure_tls_session_receive_records.c
            nx_secure_tls_verify_mac.c

        Files are modified to add buffer boundary checking in ASN.1 parsing:
            nx_secure_x509.c
            nx_secure_x509.h
            nx_secure_x509_asn1_tlv_block_parse.c
            nx_secure_x509_certificate_revocation_list_parse.c
            nx_secure_x509_crl_revocation_check.c
            nx_secure_x509_distinguished_name_parse.c
            nx_secure_x509_extended_key_usage_extension_parse.c
            nx_secure_x509_extension_find.c
            nx_secure_x509_key_usage_extension_parse.c
            nx_secure_x509_pkcs1_rsa_private_key_parse.c
            nx_secure_x509_pkcs7_decode.c
            nx_secure_x509_subject_alt_names_find.c

        Files are added to support DTLS:
            nx_secure_dtls.h
            nx_secure_dtls_api.h
            nx_secure_dtls_allocate_handshake_packet.c
            nx_secure_dtls_client_handshake.c
            nx_secure_dtls_client_protocol_version_override.c
            nx_secure_dtls_client_session_start.c
            nx_secure_dtls_hash_record.c
            nx_secure_dtls_initialize.c
            nx_secure_dtls_packet_allocate.c
            nx_secure_dtls_process_clienthello.c
            nx_secure_dtls_process_handshake_header.c
            nx_secure_dtls_process_header.c
            nx_secure_dtls_process_helloverifyrequest.c
            nx_secure_dtls_process_record.c
            nx_secure_dtls_psk_add.c
            nx_secure_dtls_receive_callback.c
            nx_secure_dtls_retransmit.c
            nx_secure_dtls_retransmit_queue_flush.c
            nx_secure_dtls_send_clienthello.c
            nx_secure_dtls_send_handshake_record.c
            nx_secure_dtls_send_helloverifyrequest.c
            nx_secure_dtls_send_record.c
            nx_secure_dtls_send_serverhello.c
            nx_secure_dtls_server_create.c
            nx_secure_dtls_server_delete.c
            nx_secure_dtls_server_handshake.c
            nx_secure_dtls_server_local_certificate_add.c
            nx_secure_dtls_server_local_certificate_remove.c
            nx_secure_dtls_server_notify_set.c
            nx_secure_dtls_server_protocol_version_override.c
            nx_secure_dtls_server_psk_add.c
            nx_secure_dtls_server_session_send.c
            nx_secure_dtls_server_session_start.c
            nx_secure_dtls_server_start.c
            nx_secure_dtls_server_stop.c
            nx_secure_dtls_server_trusted_certificate_add.c
            nx_secure_dtls_server_trusted_certificate_remove.c
            nx_secure_dtls_server_x509_client_verify_configure.c
            nx_secure_dtls_server_x509_client_verify_disable.c
            nx_secure_dtls_session_cache.c
            nx_secure_dtls_session_client_info_get.c
            nx_secure_dtls_session_create.c
            nx_secure_dtls_session_delete.c
            nx_secure_dtls_session_end.c
            nx_secure_dtls_session_local_certificate_add.c
            nx_secure_dtls_session_local_certificate_remove.c
            nx_secure_dtls_session_receive.c
            nx_secure_dtls_session_reset.c
            nx_secure_dtls_session_send.c
            nx_secure_dtls_session_start.c
            nx_secure_dtls_session_trusted_certificate_add.c
            nx_secure_dtls_session_trusted_certificate_remove.c
            nx_secure_dtls_verify_mac.c
            nxe_secure_dtls_client_protocol_version_override.c
            nxe_secure_dtls_client_session_start.c
            nxe_secure_dtls_packet_allocate.c
            nxe_secure_dtls_psk_add.c
            nxe_secure_dtls_server_create.c
            nxe_secure_dtls_server_delete.c
            nxe_secure_dtls_server_local_certificate_add.c
            nxe_secure_dtls_server_local_certificate_remove.c
            nxe_secure_dtls_server_notify_set.c
            nxe_secure_dtls_server_protocol_version_override.c
            nxe_secure_dtls_server_psk_add.c
            nxe_secure_dtls_server_session_send.c
            nxe_secure_dtls_server_session_start.c
            nxe_secure_dtls_server_start.c
            nxe_secure_dtls_server_stop.c
            nxe_secure_dtls_server_trusted_certificate_add.c
            nxe_secure_dtls_server_trusted_certificate_remove.c
            nxe_secure_dtls_server_x509_client_verify_configure.c
            nxe_secure_dtls_server_x509_client_verify_disable.c
            nxe_secure_dtls_session_client_info_get.c
            nxe_secure_dtls_session_create.c
            nxe_secure_dtls_session_delete.c
            nxe_secure_dtls_session_end.c
            nxe_secure_dtls_session_local_certificate_add.c
            nxe_secure_dtls_session_local_certificate_remove.c
            nxe_secure_dtls_session_receive.c
            nxe_secure_dtls_session_reset.c
            nxe_secure_dtls_session_send.c
            nxe_secure_dtls_session_start.c
            nxe_secure_dtls_session_trusted_certificate_add.c
            nxe_secure_dtls_session_trusted_certificate_remove.c

        Files are modified to fix compiler errors when NX_SECURE_X509_DISABLE_CRL is defined:
            nx_secure_x509.h
            nx_secure_x509_certificate_revocation_list_parse.c
            nx_secure_x509_crl_revocation_check.c
            nx_secure_x509_crl_verify.c
            nx_secure_x509_ecc_certificate_revocation_list_parse.c
            nx_secure_x509_ecc_crl_verify.c

        Files are modified to support AES-GCM cipher:
            nx_secure_crypto_method_self_test_aes.c
            nx_secure_crypto_table_self_test.c
            nx_secure_tls_record_payload_decrypt.c
            nx_secure_tls_record_payload_encrypt.c

        Files are modified to fix swapped PSK identity and hint parameters
          when sending to the remote host:
            nx_secure_tls_send_client_key_exchange.c
            nx_secure_tls_send_server_key_exchange.c

        Files are modified to fix Session ID handling problem (session ID
          length is always 0 in the initial ServerHello):
            nx_secure_tls_send_serverhello.c

        Files are modified to update scratch buffers to support the new
          default 4096-bit RSA key size:
            nx_secure_tls_process_certificate_verify.c
            nx_secure_tls_process_client_key_exchange.c
            nx_secure_tls_send_certificate_verify.c
            nx_secure_tls_send_client_key_exchange.c

        Files are modified to fix issue with processing CertificateVerify
          messages in a TLS Server with Client X.509 Certificate
          verification enabled:
            nx_secure_tls_process_remote_certificate.c
            nx_secure_tls_process_certificate_verify.c
            nx_secure_tls_remote_certificate_verify.c
            nx_secure_tls_server_handshake.c

        Files are added to support new API for allocating buffer space
          for incoming remote certificates:
            nx_secure_tls_remote_certificate_buffer_allocate.c
            nxe_secure_tls_remote_certificate_buffer_allocate.c

        Files are modified to support new API for allocating buffer space
          for incoming remote certificates:
            nx_secure_tls.h
            nx_secure_tls_api.h

        Files are added to support new API for configuring Client X.509
          authentication for TLS servers:
            nx_secure_tls_session_x509_client_verify_configure.c
            nxe_secure_tls_session_x509_client_verify_configure.c

        Files are modified to support new API for Client X.509 authentication:
            nx_secure_tls.h
            nx_secure_tls_api.h

        Files are modified to clear cryptographic secret data on errors:
            nx_secure_tls_session_receive_records.c
            nx_secure_tls_session_send.c

        Files are modified to assure that random number values are always cast
          to 32-bit type or smaller:
            nx_secure_tls_generate_premaster_secret.c

        Files are modified to assure that CBC padding values are properly checked:
            nx_secure_tls_record_payload_decrypt.c

        Files are modified to assure that MAC hash checking happens even if
          decryption padding checking fails:
            nx_secure_tls_record_payload_decrypt.c
            nx_secure_tls_process_record.c

        Files are modified to free all allocated remote certificates in a
          TLS server session as soon as certificate chain validation is
          complete. TLS clients need the remote certificates until after
          the key echange is complete so they are cleared later:
            nx_secure_tls_process_remote_certificate.c
            nx_secure_tls_send_client_key_exchange.c

        Files are modified to fix the usage of crypto metadata for hash method:
            nx_secure_tls_local_certificate_add.c
            nx_secure_tls_process_remote_certificate.c
            nx_secure_tls_remote_certificate_verify.c
            nx_secure_tls_session_create.c
            nx_secure_tls_trusted_certificate_add.c
            nx_secure_x509.h
            nx_secure_x509_certificate_verify.c
            nx_secure_x509_crl_verify.c

        Files are modified to support AEAD cipher:
            nx_secure_tls.h
            nx_secure_tls_record_payload_decrypt.c
            nx_secure_tls_record_payload_encrypt.c

        Files are modified to reset TLS session state if the TLS connection fails:
            nx_secure_tls_session_end.c
            nx_secure_tls_session_start.c

        Files are modified to return errors if cryptographic routines are
          improperly initialized:
            nx_secure_tls.h
            nx_secure_tls_finished_hash_generate.c
            nx_secure_tls_process_finished.c

        Files are modified to support elliptic curve cryptography:
            nx_secure_tls.h
            nx_secure_tls_process_client_key_exchange.c
            nx_secure_tls_process_server_key_exchange.c
            nx_secure_tls_process_serverhello_extensions.c
            nx_secure_tls_send_client_key_exchange.c
            nx_secure_tls_send_clienthello_extensions.c
            nx_secure_tls_send_server_key_exchange.c
            nx_secure_tls_send_serverhello_extensions.c
            nx_secure_dtls_process_clienthello.c
            nx_secure_dtls_send_clienthello.c
            nx_secure_dtls_send_serverhello.c

        Files are added or modified to optimize the logic:
            nx_secure_tls_client_handshake.c
            nx_secure_tls_payload_encrypt.c
            nx_secure_tls_process_clienthello_extensions.c
            nx_secure_tls_process_record.c
            nx_secure_tls_send_certificate.c
            nx_secure_tls_send_certificate_verify.c
            nx_secure_tls_send_client_key_exchange.c
            nx_secure_tls_send_clienthello.c
            nx_secure_tls_send_clienthello_extensions.c
            nx_secure_tls_send_serverhello.c
            nx_secure_tls_send_serverhello_extensions.c
            nx_secure_tls_server_handshake.c
            nx_secure_tls_session_create.c
            nx_secure_x509.c
            nx_secure_x509_certificate_chain_verify.c

        Files are modified to add wait_option to _nx_secure_tls_send_certificate:
            nx_secure_dtls_client_handshake.c
            nx_secure_dtls_server_handshake.c
            nx_secure_tls.h
            nx_secure_tls_client_handshake.c
            nx_secure_tls_send_certificate.c
            nx_secure_tls_server_handshake.c

        Files are modified to fix renegotiation issue with certain browsers due to both
          SCSV and the Secure Renegotiation extension being required (should be either/or):
            nx_secure_tls.h
            nx_secure_tls_map_error_to_alert.c
            nx_secure_tls_process_clienthello.c
            nx_secure_tls_process_clienthello_extensions.c
            nx_secure_tls_process_serverhello_extensions.c
            nx_secure_tls_session_renegotiate.c
            nx_secure_tls_session_reset.c

        Files are modified about server state processing:
            nx_secure_tls_process_clienthello.c
            nx_secure_tls_process_client_key_exchange.c
            nx_secure_tls_process_finished.c
            nx_secure_tls_server_handshake.c
            nx_secure_dtls_server_handshake.c

        Files are modified to add flexibility of using macros instead of direct C library function calls:
            nx_secure_dtls_client_handshake.c
            nx_secure_dtls_hash_record.c
            nx_secure_dtls_packet_allocate.c
            nx_secure_dtls_process_clienthello.c
            nx_secure_dtls_process_clienthello.c
            nx_secure_dtls_process_header.c
            nx_secure_dtls_process_helloverifyrequest.c
            nx_secure_dtls_process_record.c
            nx_secure_dtls_send_clienthello.c
            nx_secure_dtls_send_helloverifyrequest.c
            nx_secure_dtls_send_record.c
            nx_secure_dtls_send_serverhello.c
            nx_secure_dtls_server_handshake.c
            nx_secure_dtls_session_cache.c
            nx_secure_dtls_session_create.c
            nx_secure_dtls_session_reset.c
            nx_secure_dtls_session_send.c
            nx_secure_dtls_verify_mac.c
            nx_secure_tls.h
            nx_secure_tls_client_handshake.c
            nx_secure_tls_client_psk_set.c
            nx_secure_tls_finished_hash_generate.c
            nx_secure_tls_generate_keys.c
            nx_secure_tls_generate_premaster_secret.c
            nx_secure_tls_key_material_init.c
            nx_secure_tls_local_certificate_remove.c
            nx_secure_tls_packet_allocate.c
            nx_secure_tls_process_certificate_verify.c
            nx_secure_tls_process_changecipherspec.c
            nx_secure_tls_process_clienthello.c
            nx_secure_tls_process_client_key_exchange.c
            nx_secure_tls_process_finished.c
            nx_secure_tls_process_remote_certificate.c
            nx_secure_tls_process_serverhello.c
            nx_secure_tls_process_serverhello_extensions.c
            nx_secure_tls_psk_add.c
            nx_secure_tls_psk_find.c
            nx_secure_tls_record_payload_encrypt.c
            nx_secure_tls_send_certificate_verify.c
            nx_secure_tls_send_clienthello.c
            nx_secure_tls_send_finished.c
            nx_secure_tls_send_serverhello.c
            nx_secure_tls_session_create.c
            nx_secure_tls_session_keys_set.c
            nx_secure_tls_session_reset.c
            nx_secure_tls_session_sni_extension_parse.c
            nx_secure_tls_verify_mac.c
            nx_secure_x509_certificate_initialize.c
            nx_secure_x509_certificate_revocation_list_parse.c
            nx_secure_x509_certificate_verify.c
            nx_secure_x509_crl_revocation_check.c
            nx_secure_x509_distinguished_name_compare.c
            nx_secure_x509_dns_name_initialize.c
            nx_secure_x509_oid_parse.c

        Files are modified to add extension hook:
            nx_secure_tls.h
            nx_secure_tls_generate_keys.c
            nx_secure_tls_generate_premaster_secret.c
            nx_secure_tls_process_certificate_request.c
            nx_secure_tls_process_certificate_verify.c
            nx_secure_tls_process_clienthello.c
            nx_secure_tls_process_clienthello_extensions.c
            nx_secure_tls_process_serverhello_extensions.c
            nx_secure_tls_process_server_key_exchange.c
            nx_secure_tls_send_certificate_request.c
            nx_secure_x509.h
            nx_secure_x509_crl_verify.c

        Files are modified to properly initialize the crypto control block and pass crypto handled
        into crypto internal functions:
            nx_secure_tls.h
            nx_secure_tls_finished_hash_generate.c
            nx_secure_tls_generate_keys.c
            nx_secure_tls_handshake_hash_init.c
            nx_secure_tls_handshake_hash_update.c
            nx_secure_tls_hash_record.c
            nx_secure_tls_process_certificate_verify.c
            nx_secure_tls_process_client_key_exchange.c
            nx_secure_tls_process_serverhello_extensions.c
            nx_secure_tls_process_server_key_exchange.c
            nx_secure_tls_record_hash_initialize.c
            nx_secure_tls_record_hash_update.c
            nx_secure_x509_certificate_verify.c
            nx_secure_x509_crl_verify.c

        Files are modified to clear encryption key and other secret data:
            nx_secure_dtls_send_record.c
            nx_secure_dtls_session_end.c
            nx_secure_dtls_session_receive.c
            nx_secure_dtls_session_start.c
            nx_secure_tls_client_handshake.c
            nx_secure_tls_finished_hash_generate.c
            nx_secure_tls_generate_keys.c
            nx_secure_tls_handshake_process.c
            nx_secure_tls_hash_record.c
            nx_secure_tls_process_certificate_verify.c
            nx_secure_tls_process_record.c
            nx_secure_tls_record_hash_calculate.c
            nx_secure_tls_record_payload_encrypt.c
            nx_secure_tls_send_record.c
            nx_secure_tls_session_receive.c
            nx_secure_tls_session_receive_records.c
            nx_secure_tls_session_renegotiate.c
            nx_secure_x509_crl_verify.c

        Files are added for crypto method self test:
            nx_secure_crypto_method_self_test_3des.c
            nx_secure_crypto_method_self_test_aes.c
            nx_secure_crypto_method_self_test_des.c
            nx_secure_crypto_method_self_test_hmac_md5.c
            nx_secure_crypto_method_self_test_hmac_sha.c
            nx_secure_crypto_method_self_test_md5.c
            nx_secure_crypto_method_self_test_prf.c
            nx_secure_crypto_method_self_test_rsa.c
            nx_secure_crypto_method_self_test_sha.c
            nx_secure_crypto_rng_self_test.c
            nx_secure_crypto_table_self_test.c
            nx_secure_crypto_table_self_test.h

        Other modified files:
            nx_secure_dtls_process_clienthello.c            Adjusted the function name.
            nx_secure_dtls_send_clienthello.c               Fixed endian issue.
            nx_secure_module_hash_compute.c                 Compute the hash value of module.
            nx_secure_tls_ecc_send_client_key_exchange.c    Fixed swapped PSK identity and hint parameters.
            nx_secure_tls_process_record.c                  Removed unused function parameter (internal API).
            nx_secure_tls_session_receive_records.c         Removed unused function parameter (internal API).       
            nx_secure_tls.h                                 Supported ECC ciphersuite.
                                                            Disabled AEAD by default.
            nx_secure_tls_api.h                             Modified the format slightly.
            nx_secure_tls_check_protocol_version.c          Fix compiler errors for position independence.
            nx_secure_tls_ecc_process_certificate_request.c Fix the issue with no client certificate added.
            nx_secure_tls_finished_hash_generate.c          Fixed the function pointer checking.
            nx_secure_tls_generate_keys.c                   Rearranged code sequence for static analysis.
            nx_secure_tls_generate_premaster_secret.c       Rearranged code sequence for static analysis.
            nx_secure_tls_metadata_size_calculate.c         Fixed the usage of crypto metadata for hash method.
            nx_secure_tls_newest_supported_version.c        Moved version override support to a separate API.
            nx_secure_tls_process_certificate_verify.c      Fixed compiler warnings.
            nx_secure_tls_process_client_key_exchange.c     Used random premaster for incorrect message.
                                                              Fixed endian issue.
            nx_secure_tls_process_clienthello.c             Corrected the index of compress method.
            nx_secure_tls_process_clienthello_extensions.c  Added buffer size checking.
            nx_secure_tls_process_record.c                  Modified the format slightly.
            nx_secure_tls_process_server_key_exchange.c     Fixed endian issue.
            nx_secure_tls_process_serverhello_extensions.c  Fixed compiler warnings.
            nx_secure_tls_protocol_version_get.c            Moved version override support to a separate API. 

            nx_secure_tls_record_payload_decrypt.c          Modified the format slightly.
                                                              Fixed compiler issues.
            nx_secure_tls_record_payload_encrypt.c          Added CBC mode checking for chained packet.
                                                              Fixed packet buffer overflow while padding for AES-CBC.
            nx_secure_tls_remote_certificate_buffer_allocate.c
                                                            Rearranged code sequence for static analysis.
            nx_secure_tls_send_clienthello_extensions.c     Send ClientHello extensions.
            nx_secure_tls_send_record.c                     Skipped the hash when it was not required.
            nx_secure_tls_send_server_key_exchange.c        Fixed endian issue.
            nx_secure_tls_send_serverhello_extensions.c     Send ServerHello extensions.
            nx_secure_tls_send_serverhello.c                Fixed compiler warnings.
            nx_secure_tls_session_create.c                  Fixed the usage of crypto metadata for hash method.
            nx_secure_tls_session_iv_size_get.c             Fixed IV size for TLS 1.0.
            nx_secure_tls_session_keys_set.c                Rearranged code sequence for static analysis.
            nx_secure_tls_session_reset.c                   Optimized the logic.
            nx_secure_tls_session_time_function_set.c       Set a function pointer to get the current time.
            nx_secure_tls_verify_mac.c                      Supported zero-length TLS application data records.
            nx_secure_x509.c                                Fixed issues with parsing offsets and
                                                              version check in parsing unique IDs.
            nx_secure_x509.h                                Fixed compiler warnings.
            nx_secure_x509_certificate_revocation_list_parse.c
                                                            Improved CRL parsing.
            nx_secure_x509_certificate_initialize.c         Released mutex when return.
            nx_secure_x509_oid_parse.c                      Fix compiler errors for position independence.
            nx_secure_x509_pkcs1_rsa_private_key_parse.c    Removed obsolete comment.
            nxe_secure_tls_remote_certificate_buffer_allocate.c
                                                            Rearranged code sequence for static analysis.
            nxe_secure_tls_session_create.c                 Checked duplicate creation.
            nxe_secure_tls_session_delete.c                 Verified the session before it is deleted.

       Files renamed:

               File Name in Release 5.11                             File Name in Release 5.12
            nx_secure_tls_session_timestamp_function_set.c      nx_secure_tls_session_time_function_set.c
            nxe_secure_tls_session_timestamp_function_set.c     nxe_secure_tls_session_time_function_set.c

12/15/2017  NX Secure generic code version 5.11. This release includes the
            following major features:

        Session renegotiation and the Secure Renegotiation Indication extension
            Full TLS session renegotiation is now supported, along with the
            Secure Renegotiation Indication extension which prevents certain
            Man-in-the-Middle attacks against renegotiation.

        Certificate verification callback
            Added support to allow application to perform additional checks on
            a certificate provided by a remote host during the TLS handshake utilizing
            a user-supplied callback.

        Session callbacks for TLS Client and TLS Server
            Added support to allow the application to process TLS extensions following
            the reception of a Hello message from the remote host. This allows for
            the application to respond to extensions requiring decisions such as
            the Server Name Indication (SNI) extension which allows a client to specify
            the name of the server it wishes to connect with.

        Multiple Server Certificate support
            Added new API to support adding multiple device identity certificates,
            possibly with the same Subject (Common Name, etc.) using a new unique
            numeric identifier (supplied by the application). This allows a TLS server
            to host multiple identity certificates and choose an approriate certificate
            to send to a connecting client. Used with the TLS Server session callback,
            the application can search for the appropriate certificate during the TLS
            handshake, and the numeric identifier allows for name conflicts between
            multiple server certificates used for the same device.

        Server Name Indication TLS extension
            Added support for the TLS Server Name Indication (SNI) extension. This
            extension allows a client to specify the server with which it is trying
            to connect, allowing the server to select parameters and credentials
            in the case where a single endpoint services multiple different server
            identities.

        X.509 Certificate Revocation List parsing
            Added support for X.509 CRLs. A new API (used with the certificate verification
            callback) is provided to check revocation status for certificates recieved
            during the TLS handshake.

        X.509 Key Usage and Extended Key Usage extensions support
            Added helper functions to allow an application to check the Key Usage
            and Extended Key Usage extensions in the certificate verification callback.
            The extensions allow an application to determine if a certificate supplied
            by a remote host is valid for a particular operation (e.g. identifying a
            TLS server).

        DNS verification
            Added support for checking the URL top-level domain name for a remote host
            against the certificate provided by that remote host. A new API (used
            with the certificate verification callback) is provided to compare the TLD
            name of the URL used to access a host with the Common Name and subjectAltName
            DNSName fields in the X.509 certificate provided by the remote host.

        Vendor-defined private key types
            Added support for private key types beyond the standard DER-encoded RSA
            keys. Vendor-defined private keys will be passed directly to the hardware
            driver for the appropriate cryptographic operations, with the assumption
            that all parsing, unwrapping, and processing of those keys will be handled
            by the hardware or hardware driver.

        Files are modified or added to add session callbacks:
            nx_secure_tls.h
            nx_secure_tls_process_clienthello.c
            nx_secure_tls_session_client_callback_set.c
            nx_secure_tls_process_serverhello.c
            nx_secure_tls_session_server_callback_set.c
            nxe_secure_tls_session_client_callback_set.c
            nxe_secure_tls_session_server_callback_set.c

        Files are modified to match the certificate ID:
            nx_secure_tls_local_certificate_remove.c
            nx_secure_tls_trusted_certificate_remove.c
            nx_secure_x509.h
            nx_secure_x509_certificate_chain_verify.c
            nx_secure_x509_certificate_list_add.c
            nx_secure_x509_certificate_list_find.c
            nx_secure_x509_certificate_list_remove.c
            nx_secure_x509_local_device_certificate_get.c
            nx_secure_x509_store_certificate_find.c
            nx_secure_x509_store_certificate_add.c
            nx_secure_x509_store_certificate_remove.c

        Files are modified or added to support multiple extensions:
            nx_secure_tls.h
            nx_secure_tls_process_clienthello.c
            nx_secure_tls_process_clienthello_extensions.c
            nx_secure_tls_process_serverhello.c
            nx_secure_tls_process_serverhello_extensions.c
            nx_secure_tls_session_sni_extension_parse.c
            nx_secure_tls_session_sni_extension_set.c
            nxe_secure_tls_session_sni_extension_parse.c
            nxe_secure_tls_session_sni_extension_set.c

        Files are modified or added to support DTLS:
            nxe_secure_dtls_packet_allocate.c
            nxe_secure_dtls_session_create.c
            nxe_secure_dtls_session_delete.c
            nxe_secure_dtls_session_end.c
            nxe_secure_dtls_session_receive.c
            nxe_secure_dtls_session_reset.c
            nxe_secure_dtls_session_send.c
            nxe_secure_dtls_session_start.c
            nx_secure_dtls_allocate_handshake_packet.c
            nx_secure_dtls_api.h
            nx_secure_dtls_client_handshake.c
            nx_secure_dtls.h
            nx_secure_dtls_hash_record.c
            nx_secure_dtls_packet_allocate.c
            nx_secure_dtls_process_clienthello.c
            nx_secure_dtls_process_handshake_header.c
            nx_secure_dtls_process_header.c
            nx_secure_dtls_process_helloverifyrequest.c
            nx_secure_dtls_process_record.c
            nx_secure_dtls_retransmit.c
            nx_secure_dtls_retransmit_queue_flush.c
            nx_secure_dtls_send_clienthello.c
            nx_secure_dtls_send_handshake_record.c
            nx_secure_dtls_send_helloverifyrequest.c
            nx_secure_dtls_send_record.c
            nx_secure_dtls_send_serverhello.c
            nx_secure_dtls_server_handshake.c
            nx_secure_dtls_session_cache.c
            nx_secure_dtls_session_create.c
            nx_secure_dtls_session_delete.c
            nx_secure_dtls_session_end.c
            nx_secure_dtls_session_receive.c
            nx_secure_dtls_session_reset.c
            nx_secure_dtls_session_send.c
            nx_secure_dtls_session_start.c
            nx_secure_dtls_verify_mac.c
            nx_secure_tls_check_protocol_version.c
            nx_secure_tls_finished_hash_generate.c
            nx_secure_tls_handshake_hash_update.c
            nx_secure_tls_newest_supported_version.c
            nx_secure_tls_process_certificate_request.c
            nx_secure_tls_send_certificate_verify.c

        Files are modified or added to optimize the logic:
            nx_secure_tls_client_handshake.c
            nx_secure_tls_ciphersuite_lookup.c
            nx_secure_tls_finished_hash_generate.c
            nx_secure_tls_generate_keys.c
            nx_secure_tls_handshake_process.c
            nx_secure_tls_key_material_init.c
            nx_secure_tls_process_certificate_request.c
            nx_secure_tls_process_certificate_verify.c
            nx_secure_tls_process_changecipherspec.c
            nx_secure_tls_process_header.c
            nx_secure_tls_process_remote_certificate.c
            nx_secure_tls_send_alert.c
            nx_secure_tls_send_certificate.c
            nx_secure_tls_send_certificate_verify.c
            nx_secure_tls_send_changecipherspec.c
            nx_secure_tls_send_clienthello.c
            nx_secure_tls_send_record.c
            nx_secure_tls_send_server_key_exchange.c
            nx_secure_tls_send_serverhello.c
            nx_secure_tls_server_handshake.c
            nx_secure_tls_session_create.c
            nx_secure_tls_session_iv_size_get.c
            nx_secure_tls_session_keys_set.c
            nx_secure_tls_session_start.c
            nx_secure_tls_verify_mac.c
            nx_secure_x509_certificate_list_find.c
            nx_secure_x509_remote_endpoint_certificate_get.c
            nx_secure_x509_store_certificate_add.c
            nxe_secure_tls_packet_allocate.c
            nxe_secure_tls_session_create.c

        Files are modified or added to support renegotiation:
            nx_secure_tls.h
            nx_secure_tls_client_handshake.c
            nx_secure_tls_process_clienthello.c
            nx_secure_tls_process_finished.c
            nx_secure_tls_send_clienthello.c
            nx_secure_tls_send_finished.c
            nx_secure_tls_send_hellorequest.c
            nx_secure_tls_send_serverhello.c
            nx_secure_tls_session_receive.c
            nx_secure_tls_session_receive_records.c
            nx_secure_tls_session_renegotiate.c
            nx_secure_tls_session_renegotiation_callback_set.c
            nx_secure_tls_session_reset.c
            nxe_secure_tls_session_renegotiate.c
            nxe_secure_tls_session_renegotiation_callback_set.c

        Files are modified to support more ciphersuites:
            nx_secure_tls.h
            nx_secure_tls_process_clienthello.c
            nx_secure_tls_record_payload_decrypt.c
            nx_secure_tls_record_payload_encrypt.c
            nx_secure_tls_send_certificate_verify.c
            nx_secure_tls_send_client_key_exchange.c
            nx_secure_tls_send_clienthello.c

        Files are modified to support ECJPAKE for DTLS:
            nx_secure_tls.h
            nx_secure_tls_generate_keys.c
            nx_secure_tls_generate_premaster_secret.c
            nx_secure_tls_process_client_key_exchange.c
            nx_secure_tls_process_server_key_exchange.c
            nx_secure_tls_psk_add.c
            nx_secure_tls_record_payload_decrypt.c
            nx_secure_tls_send_client_key_exchange.c
            nx_secure_tls_send_server_key_exchange.c
            nxe_secure_tls_psk_add.c

        Files are modified to fix compiler warnings:
            nx_secure_tls_process_clienthello.c
            nx_secure_tls_record_payload_decrypt.c
            nx_secure_tls_remote_certificate_verify.c
            nx_secure_tls_send_certificate.c
            nx_secure_tls_session_end.c
            nx_secure_x509.c
            nx_secure_x509_certificate_chain_verify.c
            nx_secure_x509_certificate_list_add.c
            nx_secure_x509_certificate_list_find.c
            nx_secure_x509_certificate_list_remove.c
            nx_secure_x509_certificate_verify.c
            nx_secure_x509_remote_endpoint_certificate_get.c

        Files are modified to support TLS protocol version override:
            nx_secure_tls_check_protocol_version.c
            nx_secure_tls_newest_supported_version.c
            nx_secure_tls_session_reset.c

        Files are modified to support context specific ASN.1 tags:
            nx_secure_x509_asn1_tlv_block_parse.c
            nx_secure_x509_pkcs1_rsa_private_key_parse.c
            nx_secure_x509_pkcs7_decode.c

        Files are added to hash record:
            nx_secure_tls_record_hash_calculate.c
            nx_secure_tls_record_hash_initialize.c
            nx_secure_tls_record_hash_update.c

        Files are added to free remote certificate:
            nx_secure_tls_remote_certificate_free.c
            nx_secure_tls_remote_certificate_free_all.c
            nxe_secure_tls_remote_certificate_free_all.c

        Files are added to support vendor-defined private key type:
            nx_secure_tls_process_client_key_exchange.c
            nx_secure_tls_send_certificate_verify.c
            nx_secure_tls_send_client_key_exchange.c
            nx_secure_x509.c
            nx_secure_x509_certificate_initialize.c
            nx_secure_x509_certificate_verify.c
            nx_secure_x509_pkcs1_rsa_private_key_parse.c
            nxe_secure_x509_certificate_initialize.c

        Files are added to support empty certificate messages:
            nx_secure_tls_send_certificate.c
            nx_secure_tls_send_certificate_verify.c

        Files are added for server certificate API:
            nx_secure_tls_server_certificate_add.c
            nx_secure_tls_server_certificate_find.c
            nx_secure_tls_server_certificate_remove.c
            nxe_secure_tls_server_certificate_add.c
            nxe_secure_tls_server_certificate_find.c
            nxe_secure_tls_server_certificate_remove.c

        Files are modified to add compare bit fields:
            nx_secure_tls_send_certificate.c
            nx_secure_x509.h
            nx_secure_x509_certificate_chain_verify.c
            nx_secure_x509_certificate_list_remove.c
            nx_secure_x509_distinguished_name_compare.c
            nx_secure_x509_remote_endpoint_certificate_get.c

        Other modified files:
            nx_secure_tls.h                                 Supported to configure symbols for PSK buffers.
                                                            Sorted the functions by alphabet order.
            nx_secure_tls_api.h                             Sorted the API by alphabet order.
            nx_secure_tls_check_protocol_version.c          Supported TLS protocol version override.
            nx_secure_tls_client_handshake.c                Handled the return values.
                                                            Fixed packet leak issue.
            nx_secure_tls_initialize.c                      Fixed the function name.
            nx_secure_port.h                                Fixed a typo.
            nx_secure_tls_generate_keys.c                   Added additional check for missing PRF.
            nx_secure_tls_map_error_to_alert.c              Updated mapping for certificate
                                                              verification failure.
            nx_secure_tls_process_client_key_exchange.c     Updated to new crypto API.
            nx_secure_tls_process_record.c                  Fixed record length check.
            nx_secure_tls_psk_find.c                        Returned the first associated PSK when
                                                              no hint is received.
            nx_secure_tls_record_payload_encrypt.c          Supported data in packet chain.
            nx_secure_tls_remote_certificate_verify.c       Added expiration data check.
            nx_secure_tls_send_certificate.c                Released mutex before block operation.
            nx_secure_tls_send_clienthello.c                Supported extensions.
            nx_secure_tls_send_handshake_record.c           Released packet on error.
            nx_secure_tls_send_record.c                     Let the caller release packet when TCP send failed.
            nx_secure_tls_send_serverhello.c                Supported extensions.
            nx_secure_tls_server_handshake.c                Added support for server-side client
                                                              certificate verification.
                                                            Released packet on send failed.
            nx_secure_tls_session_end.c                     Improved TLS session shutdown.
                                                            Released packet on send failed.
            nx_secure_tls_session_reset.c                   Cleaned up existing certificate.
            nx_secure_x509.c                                Added support for extension parsing.
            nx_secure_x509.h                                Added support for extension parsing and fixed typos for API names.
            nx_secure_x509_certificate_list_add.c           Fixed the logic to add certificate with duplicate ID.
            nx_secure_x509_distinguished_name_compare.c     Supported empty name comparison.
                                                            Improved the distinguished name comparison logic.
            nx_secure_x509_find_certificate_methods.c       Updated API to take an ID for the
                                                              algorithm desired.
            nx_secure_x509_local_device_certificate_get.c   Used pre-defined symbols instead of
                                                              hardcoded values.
            nx_secure_x509_pkcs7_decode.c                   Fixed signature length check.

        Other added files:
            nx_secure_tls_active_certificate_set.c          Set the active local certificate.
            nx_secure_tls_local_certificate_find.c          New API to find certificates by Common Name.
            nx_secure_tls_send_certificate_request.c        Populated an NX_PACKET with a CertificateRequest message.
            nx_secure_tls_session_certificate_callback_set.c
                                                            Set up a function pointer that TLS will
                                                              invoke when a certificate is received.
            nx_secure_tls_session_client_verify_disable.c   Disabled client certificate verification.
            nx_secure_tls_session_client_verify_enable.c    Enabled client certificate verification.
            nx_secure_tls_session_protocol_version_override.c
                                                            Overrided the TLS protocol version.
            nx_secure_x509_certificate_revocation_list_parse.c
                                                            Supported revocation list parse.
            nx_secure_x509_common_name_dns_check.c          Checked a certificate's Common Name
                                                              against a Top Level Domain name (TLD).
            nx_secure_x509_crl_revocation_check.c           Supported revocation check.
            nx_secure_x509_crl_verify.c                     Verified CRL.
            nx_secure_x509_distinguished_name_parse.c       Parsed a DER-encoded X.509 Distinguished Name.
            nx_secure_x509_dns_name_initialize.c            Initialized an X.509 DNS name.
            nx_secure_x509_expiration_check.c               Checked a certificate's validity period.
            nx_secure_x509_extended_key_usage_extension_parse.c
                                                            Parse through an X.509 certificate for
                                                              an Extended KeyUsage extension.
            nx_secure_x509_extension_find.c                 Added support for X.509 extension parsing.
            nx_secure_x509_key_usage_extension_parse.c      Parse through an X.509 certificate for
                                                              a KeyUsage extension.
            nx_secure_x509_local_certificate_find.c         Searched a given certificate store.
            nx_secure_x509_oid_parse.c                      Parsed a DER-encoded Object Identifier string.
            nx_secure_x509_subject_alt_names_find.c         Searched X.509 subjectAltName.
            nx_secure_x509_wildcard_compare.c               Compared wildcard names.
            nxe_secure_tls_active_certificate_set.c         Set the active local certificate.
            nxe_secure_tls_local_certificate_find.c         New API to find local certificate by Common Name.
            nxe_secure_tls_session_certificate_callback_set.c
                                                            Set up a function pointer that TLS will
                                                              invoke when a certificate is received.
            nxe_secure_tls_session_client_verify_disable.c  Disabled client certificate verification.
            nxe_secure_tls_session_client_verify_enable.c   Enabled client certificate verification.
            nxe_secure_tls_session_protocol_version_override.c
                                                            Overrided the TLS protocol version.
            nxe_secure_x509_common_name_dns_check.c         Checked a certificate's Common Name
                                                              against a Top Level Domain name (TLD).
            nxe_secure_x509_crl_revocation_check.c          Supported revocation check.
            nxe_secure_x509_dns_name_initialize.c           Initialized an X.509 DNS name.
            nxe_secure_x509_extension_find.c                Added support for X.509 extension parsing.
            nxe_secure_x509_extended_key_usage_extension_parse.c
                                                            Parse through an X.509 certificate for
                                                              an Extended KeyUsage extension.
            nxe_secure_x509_key_usage_extension_parse.c     Parse through an X.509 certificate for
                                                              a KeyUsage extension.

        Files renamed:

               File Name in Release 5.10                             File Name in Release 5.11
            nx_secure_tls_process_server_certificate.c      nx_secure_tls_process_remote_certificate.c

06/09/2017  Initial NX Secure generic code version 5.10.


Copyright(c) 1996-2019 Express Logic, Inc.


Express Logic, Inc.
11423 West Bernardo Court
San Diego, CA  92127

www.expresslogic.com

