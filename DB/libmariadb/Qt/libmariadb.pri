
INCLUDEPATH +=  ../../mariadb/plugins/auth \
                ../../mariadb/plugins/auth/ref10

SOURCES += ../../mariadb/libmariadb/get_password.c \
           ../../mariadb/plugins/auth/auth_gssapi_client.c \
           ../../mariadb/plugins/auth/caching_sha2_pw.c \
           ../../mariadb/plugins/auth/dialog.c \
           ../../mariadb/plugins/auth/ed25519.c \
           ../../mariadb/plugins/auth/mariadb_cleartext.c \
           ../../mariadb/plugins/auth/my_auth.c \
           ../../mariadb/plugins/auth/old_password.c \
           ../../mariadb/plugins/auth/sha256_pw.c \
           ../../mariadb/plugins/auth/sspi_client.c \
           ../../mariadb/plugins/auth/sspi_errmsg.c \
           ../../mariadb/plugins/pvio/pvio_npipe.c \
           ../../mariadb/plugins/pvio/pvio_shmem.c \
           ../../mariadb/plugins/pvio/pvio_socket.c \
           ../../mariadb/win-iconv/win_iconv.c \
           ../../mariadb/libmariadb/mariadb_async.c \
           ../../mariadb/libmariadb/mariadb_charset.c \
           ../../mariadb/libmariadb/mariadb_dyncol.c \
           ../../mariadb/libmariadb/mariadb_lib.c \
           ../../mariadb/libmariadb/mariadb_rpl.c \
           ../../mariadb/libmariadb/mariadb_stmt.c \
           ../../mariadb/libmariadb/ma_alloc.c \
           ../../mariadb/libmariadb/ma_array.c \
           ../../mariadb/libmariadb/ma_context.c \
           ../../mariadb/libmariadb/ma_charset.c \
           ../../mariadb/libmariadb/ma_compress.c \
           ../../mariadb/libmariadb/ma_default.c \
           ../../mariadb/libmariadb/ma_dtoa.c \
           ../../mariadb/libmariadb/ma_errmsg.c \
           ../../mariadb/libmariadb/ma_hash.c \
           ../../mariadb/libmariadb/ma_init.c \
           ../../mariadb/libmariadb/ma_io.c \
           ../../mariadb/libmariadb/ma_list.c \
           ../../mariadb/libmariadb/ma_ll2str.c \
           ../../mariadb/libmariadb/ma_loaddata.c \
           ../../mariadb/libmariadb/ma_net.c \
           ../../mariadb/libmariadb/ma_password.c \
           ../../mariadb/libmariadb/ma_pvio.c \
           ../../mariadb/libmariadb/ma_sha1.c \
           ../../mariadb/libmariadb/ma_stmt_codec.c \
           ../../mariadb/libmariadb/ma_string.c \
           ../../mariadb/libmariadb/ma_time.c \
           ../../mariadb/libmariadb/ma_tls.c \
           ../../mariadb/libmariadb/win32_errmsg.c \
           ../../mariadb/libmariadb/ma_client_plugin.c \
           ../../mariadb/zlib/adler32.c \
           ../../mariadb/zlib/compress.c \
           ../../mariadb/zlib/crc32.c \
           ../../mariadb/zlib/deflate.c \
           ../../mariadb/zlib/gzclose.c \
           ../../mariadb/zlib/gzlib.c \
           ../../mariadb/zlib/gzread.c \
           ../../mariadb/zlib/gzwrite.c \
           ../../mariadb/zlib/infback.c \
           ../../mariadb/zlib/inffast.c \
           ../../mariadb/zlib/inflate.c \
           ../../mariadb/zlib/inftrees.c \
           ../../mariadb/zlib/trees.c \
           ../../mariadb/zlib/uncompr.c \
           ../../mariadb/zlib/zutil.c

