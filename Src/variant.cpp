/*
 * variant.cpp
 *
 *  Created on: May 27, 2017
 *      Author: External
 */


#include "variant.h"


CVarint::CVarint(int value) {
	this->content = value;
}


CVariant	*CVarint::write (const char *txt){
	this->content = atoi(txt);
	return this;
}

char		*CVarint::tostring (){
	return itoa(this->content, resultado, 10);
}


CVardec::CVardec(double value) {
	this->content = value;
}


CVariant	*CVardec::write (const char *txt){
	this->content = atof(txt);
	return this;
}

char		*CVardec::tostring (){
	// 1 decimal place ad-doc.
	//	Easy to generalize to n, changin 10 and de format specifier

//	char	*resultado = (char *) malloc(8);

	int intp = (int) content;
	int fracp = (int) ((content - intp)* 10 );
	if (fracp < 0) fracp = -fracp;
	sprintf(resultado, "%d.%1d", intp, fracp);
	return resultado;
}



CVarstring::CVarstring(const char *value) {

#ifdef TFIJO
	if (value)
		strncpy (this->content,value, TFIJO-1);
	else 
		this->content[0] = 0;
#else
	if (value)
		this->content = strdup(value);
	else 
		this->content = NULL;
#endif

}

CVarstring::~CVarstring() {
#ifdef TFIJO
#else
	if (this->content ) {
		free (this->content);
	}
#endif
}


CVariant	*CVarstring::write (const char *txt){
#ifdef TFIJO
	strncpy (this->content,txt, TFIJO-1);
//	strcpy (this->content,txt);
#else
	if (this->content) 
		free (this->content);
	this->content = strdup(txt);
#endif
	return this;
}


char		*CVarstring::tostring (){

	if (this->content) {
//		char *resultado = (char *) malloc (strlen(this->content) +1);
#ifdef	TFIJO
		strcpy (resultado, this->content);
		return resultado;
#else
		return this->content;
#endif

	}
	else {
//		char *resultado = (char *) malloc (1);
#ifdef	TFIJO
		strcpy (resultado, "");
		return resultado;
#else
		return NULL;
#endif


	}
}

CVartuple::CVartuple (char sep, CVariant *v1, CVariant *v2){
	this->separator  = sep;
	this->parts[0] = v1;
	this->parts[1] = v2;
}

CVartuple::~CVartuple() {
	delete parts[0];
	delete parts[1];
}


CVariant	*CVartuple::write (const char *txt){
	char *p = strchr(txt, this->separator);
	if (p) {
		*p = 0;
		p++;
	}
	if (this->parts[0]!= NULL) 
		this->parts[0]->write(txt);
	else
		this->parts[0] = new CVarstring(txt);

	if (p) {
		if (this->parts[1]) 
			this->parts[1]->write(p);
		else
			this->parts[1] = new CVarstring(p);
	}
	return this;
}


char		*CVartuple::tostring (){
	int longitud = 0;
	char *t1;
	if (this->parts[0]) {
		t1 = this->parts[0]->tostring();
		longitud += strlen(t1);
	}
	else {
		t1 = NULL;
	}
	char *t2;
	if (this->parts[1]) {
		t2 = this->parts[1]->tostring();
		longitud += strlen(t2);
	}
	else {
		t2 = NULL;
	}
	char *resultado = (char *) malloc (longitud + 4);
	sprintf (resultado, "(%s,%s)", t1?t1:"", t2?t2:"" ); 
	if (t1) free (t1);
	if (t2) free (t2);
	return resultado;
}


CVarlist::CVarlist(char sep, CVariant **elems, bool tupflag){
	this->separator  = sep;
	this->flag = tupflag;
	unsigned int j = 0 ;
	if (elems) 
		do  j++; while (elems[j]);
	this->number = j;
	if (this->number) {
		this->parts = new CVariant *[j+1];
		for (unsigned int i = 0; i < j; i++) {
			this->parts[i] = elems ? elems[i] : NULL;
		}
	}
	else {
		this->parts = NULL;
	}



}


CVariant	*CVarlist::write (const char *txt){
	const char *p= txt;
	const char *s = p;
	unsigned int i = 0;
	if (this->parts) {
		while (i < this->number) {
			delete this->parts[i];
			i++;
		}
		delete [] this->parts;
	}	
	for (i=0; s[i]; s[i]==';' ? i++ : *s++);
; 
		

	if (this->flag) {
		this->number = i / 2;
		this->parts = new CVariant *[number];
		char *q;
		i = 0;
		while (i < this->number ) {
			q = strchr(p, this->separator);
			if (q) {
				q = strchr(q+1, this->separator);
				*q = 0;
			}
			this->parts[i] = new CVartuple(this->separator);
			this->parts[i]->write (p);
			p = q + 1;
			i++;
		}
	}
	else {
		this->number = i;
		this->parts = new CVariant *[number];
		char *q; 
		i = 0;
		while ((q = strchr(p, this->separator)) && i < this->number ) {
			*q = 0;
			this->parts[i] = new CVarstring(p);
			i++;
			p = q+1;
		}
	}
	return this;
}


char		*CVarlist::tostring (){
	char *resultado = (char *) malloc (1024) ; // OJO: ÑAPA!!!
	strcpy (resultado, "(");
	unsigned i = 0;
	while (i < this->number) {
		if (i > 0) 
			strcat (resultado, ",");
		if (this->parts[i]) {
			char *x = this->parts[i]->tostring();
			strcat (resultado, x);
			free (x);
		}
		i++;
	}
	strcat (resultado, ")");
	return resultado;
}


CVarlist2::CVarlist2(char sep, CVariant **elems): CVarlist(sep,elems, true) {
}


