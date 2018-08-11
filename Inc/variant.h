/*
 * variant.h
 *
 *  Created on: May 27, 2017
 *      Author: External
 */

#ifndef VARIANT_H_
#define VARIANT_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>




#undef		TFIJO


/***************************************************
	Base class (abstract)
	just define the class interface
****************************************************/
class CVariant	{
	public :
		virtual CVariant	*write (const char *txt) = 0;
		virtual char		*tostring () = 0;
		virtual				~CVariant()		{};
	protected :
#ifdef	TFIJO
		char				resultado[TFIJO];
#else

#endif


							CVariant() {};

};


/***************************************************
	class CVarint (integer content)
****************************************************/
class CVarint : public CVariant {
	public :
					CVarint(int value = 0);
		CVariant	*write (const char *txt);
		char		*tostring ();
	private :
		char		resultado[16];
		int			content;
};


/***************************************************
	class CVardec (decimal content)
****************************************************/
class CVardec : public CVariant {
	public :
					CVardec(double value = 0.0);
		CVariant	*write (const char *txt);
		char		*tostring ();
	private :
		double		content;
		char		resultado[16];
};


/***************************************************
	class CVarstring (trext content)
****************************************************/
class CVarstring : public CVariant {
	public :
					CVarstring(const char *value = NULL);
					~CVarstring();
		CVariant	*write (const char *txt);
		char		*tostring ();
	private :
#ifdef TFIJO
		char		content[TFIJO];
#else
		char		*content;
#endif
};


/***************************************************
	class CVartuple (tuple of Variants)
****************************************************/
class CVartuple : public CVariant {
	public :
						CVartuple(char sep, CVariant *v1 = NULL, CVariant *v2 = NULL );
						~CVartuple();
		CVariant		*write (const char *txt);
		char			*tostring ();
	private :
		char			separator;
		CVariant		*parts[2];
};


/***************************************************
	class CVarlist (variable size list os Variants)
****************************************************/
class CVarlist : public CVariant {
	public :
						CVarlist(char sep, CVariant **velems = NULL, bool tupflag = false );
		CVariant		*write (const char *txt);
		char			*tostring ();
	private :
		char			separator;
		bool			flag;
		unsigned int	number;
		CVariant		**parts;
};


/***************************************************
	CLASE VARIANT CVarlist2 (id for duples)  // ad-hoc
****************************************************/
class CVarlist2 : public CVarlist {
	public :
						CVarlist2(char sep, CVariant **velems = NULL);
	//	CVariant		*write (const char *txt);
};





#endif /* VARIANT_H_ */
