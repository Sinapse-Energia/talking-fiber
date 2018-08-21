#ifndef MQTTAPI_H_
#define MQTTAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

extern	int		MqttConnect		(const char	*host, unsigned int port, char *username, char *password);

extern	int		MqttSubscribe	(int handle, char *topic);

extern	char	*MqttGetMessage	(int handle, char *buf, int bufsize, char* topicbuf, int topicbufsize); // fgets style

//extern	int		Subscribe		(int handle, char *topic);

extern	int		MqttPutMessage	(int handle, char *topic, char *message);



extern	int		MqttDisconnect	(int handle);

extern	int		MqttPing(int handle);


extern int		tprintf(int hcon, char *texto,...);



extern	char	topictr[];
extern	char	topicmqdebug[];
extern	char	topicrfdebug[];

#ifdef __cplusplus
}
#endif

#endif

