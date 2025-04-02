#include "network.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

const char* userAgent = "3DShock/0.0.0";
char token[] = "SBB7r75ogF1Iul8CKPVgaBAFy62MIFdhCSIOuF4Moyom421YQICZLUS0vo8vvrp7";
User user = {token};

Result http_get(const char *url, u8* buf, char *token)
{
	Result ret=0;
	httpcContext context;
	char *newurl=nullptr;
	u32 statuscode=0;
	u32 contentsize=0, readsize=0, size=0;
	u8 *lastbuf;

	printf("Downloading %s\n",url);

	do {
		ret = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 1);
		printf("%i", ret);
		printf("return from httpcOpenContext: %" PRId32 "\n",ret);

		// This disables SSL cert verification, so https:// will be usable
		ret = httpcSetSSLOpt(&context, SSLCOPT_DisableVerify);
		printf("return from httpcSetSSLOpt: %" PRId32 "\n",ret);

		// Enable Keep-Alive connections
		ret = httpcSetKeepAlive(&context, HTTPC_KEEPALIVE_ENABLED);
		printf("return from httpcSetKeepAlive: %" PRId32 "\n",ret);

		// Set a User-Agent header so websites can identify your application
		ret = httpcAddRequestHeaderField(&context, "User-Agent", userAgent);
		printf("return from httpcAddRequestHeaderField: %" PRId32 "\n",ret);

		// Tell the server we can support Keep-Alive connections.
		// This will delay connection teardown momentarily (typically 5s)
		// in case there is another request made to the same server.
		ret = httpcAddRequestHeaderField(&context, "Connection", "Keep-Alive");
		printf("return from httpcAddRequestHeaderField: %" PRId32 "\n",ret);


		//ret = httpcAddRequestHeaderField(&context, "Host", "api.openshock.zap");
		//printf("return from httpcAddRequestHeaderField: %" PRId32 "\n",ret);

		ret = httpcAddRequestHeaderField(&context, "OpenShockToken", token);
		printf("return from httpcAddRequestHeaderField: %" PRId32 "\n",ret);

		ret = httpcBeginRequest(&context);
		if(ret!=0){
			httpcCloseContext(&context);
			if(!newurl) free(newurl);
			return ret;
		}

		ret = httpcGetResponseStatusCode(&context, &statuscode);
		if(ret!=0){
			httpcCloseContext(&context);
			if(!newurl) free(newurl);
			return ret;
		}

		if ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308)) {
			if(!newurl) newurl = static_cast<char *>(malloc(0x1000)); // One 4K page for new URL
			if (!newurl){
				httpcCloseContext(&context);
				return -1;
			}
			ret = httpcGetResponseHeader(&context, "Location", newurl, 0x1000);
			url = newurl; // Change pointer to the url that we just learned
			printf("redirecting to url: %s\n",url);
			httpcCloseContext(&context); // Close this context before we try the next
		}
	} while ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308));

	if(statuscode!=200){
		printf("URL returned status: %" PRId32 "\n", statuscode);
		httpcCloseContext(&context);
		if(newurl) free(newurl);
		return -2;
	}

	// This relies on an optional Content-Length header and may be 0
	ret=httpcGetDownloadSizeState(&context, nullptr, &contentsize);
	if(ret!=0){
		httpcCloseContext(&context);
		if(newurl) free(newurl);
		return ret;
	}

	printf("reported size: %" PRId32 "\n",contentsize);

	// Start with a single page buffer
	buf = static_cast<u8 *>(malloc(0x1000));
	if(!buf){
		httpcCloseContext(&context);
		if(newurl) free(newurl);
		return -1;
	}

	do {
		// This download loop resizes the buffer as data is read.
		ret = httpcDownloadData(&context, buf+size, 0x1000, &readsize);
		size += readsize;
		if (ret == static_cast<s32>(HTTPC_RESULTCODE_DOWNLOADPENDING)){
				lastbuf = buf; // Save the old pointer, in case realloc() fails.
				buf = static_cast<u8 *>(realloc(buf, size + 0x1000));
				if(!buf){
					httpcCloseContext(&context);
					free(lastbuf);
					if(newurl) free(newurl);
					return -1;
				}
			}
	} while (ret == static_cast<s32>(HTTPC_RESULTCODE_DOWNLOADPENDING));

	if(ret!=0){
		httpcCloseContext(&context);
		if(newurl) free(newurl);
		free(buf);
		return -1;
	}

	// Resize the buffer back down to our actual final size
	lastbuf = buf;
	buf = static_cast<u8 *>(realloc(buf, size));
	if(!buf){ // realloc() failed.
		httpcCloseContext(&context);
		free(lastbuf);
		if(newurl) free(newurl);
		return -1;
	}

	printf("downloaded size: %" PRId32 "\n",size);

	httpcCloseContext(&context);
	free(buf);
	if (newurl) free(newurl);

	return 0;
}

void getShockers() {
	u8* buf;
	http_get("http://devkitpro.org/misc/httpexample_rawimg.rgb", buf, user.token);
	if (buf) {
		printf("Buffer: %s\n", buf);
		free(buf);
	} else {
		printf("Failed to get shockers\n");
	}
}