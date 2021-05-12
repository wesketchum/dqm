//#include "Hist.hpp"
#include <librdkafka>

void Exporter(int test)
{
  std::cout << "test = " << test << std::endl;

  char hostname[128];
  char errstr[512];
  
  rd_kafka_conf_t *conf = rd_kafka_conf_new();

  //Error handling
  //--------------
  if (gethostname(hostname, sizeof(hostname))) 
  {
    fprintf(stderr, "%% Failed to lookup hostname\n");
    exit(1);
  }
  
  //if (rd_kafka_conf_set(conf, "client.id", hostname, errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) 
  if (rd_kafka_conf_set(conf, "client.id", "myid", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) 
  { 
    fprintf(stderr, "%% %s\n", errstr);
    exit(1);
  }
  
  //if (rd_kafka_conf_set(conf, "bootstrap.servers", "host1:9092,host2:9092", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) 
  if (rd_kafka_conf_set(conf, "bootstrap.servers", "dbnile-kafka-a-5.cern.ch:9443", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) 
  {
    fprintf(stderr, "%% %s\n", errstr);
    exit(1);
  }
  
  rd_kafka_topic_conf_t *topic_conf = rd_kafka_topic_conf_new();
  
  if (rd_kafka_topic_conf_set(topic_conf, "acks", "all",
                        errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
    fprintf(stderr, "%% %s\n", errstr);
    exit(1);
  }

  //Pip's additions
  if (rd_kafka_conf_set(conf, "security.protocol", "SSL", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) 
  { 
    fprintf(stderr, "%% %s\n", errstr);
    exit(1);
  }
  
  if (rd_kafka_conf_set(conf, "ssl_ca", "/afs/cern.ch/user/p/pahamilt/certs/cerncacerts.pem", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) 
  { 
    fprintf(stderr, "%% %s\n", errstr);
    exit(1);
  }
  
  if (rd_kafka_conf_set(conf, "ssl.certificate.pem", "/afs/cern.ch/user/p/pahamilt/certs/myCertificate.crt.pem", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) 
  { 
    fprintf(stderr, "%% %s\n", errstr);
    exit(1);
  }
 
  if (rd_kafka_conf_set(conf, "ssl.key.pem", "/afs/cern.ch/user/p/pahamilt/certs/myCertificate.key.pem", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) 
  { 
    fprintf(stderr, "%% %s\n", errstr);
    exit(1);
  }
   
  //Create Kafka producer handle
  //----------------------------
  rd_kafka_t *rk;
  if (!(rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr)))) 
  {
    fprintf(stderr, "%% Failed to create new producer: %s\n", errstr);
    exit(1);
  }

  rd_kafka_topic_t *rkt = rd_kafka_topic_new(rk, "testdunedqm", topic_conf);
  
  if (rd_kafka_produce(rkt, RD_KAFKA_PARTITION_UA, RD_KAFKA_MSG_F_COPY, payload, payload_len, key, key_len, NULL) == -1) 
  {
    fprintf(stderr, "%% Failed to produce to topic %s: %s\n", "testdunedqm", rd_kafka_err2str(rd_kafka_errno2err(errno)));
  }

}
