/*
 * NBinterface.h
 *
 *  Created on: May 26, 2017
 *
 *      Author: External
 *
 *
 *		This file declares the C++ functions with C storage
 *		providing entry points to the Nothbound subsystem
 *		form de main program and/or all the remainder C stuff
 *
 */

#ifndef NBINTERFACE_H_
#define NBINTERFACE_H_



#ifdef __cplusplus
extern "C" {
#endif



	int		CreateContext	();
	int		ReadMetadata	(char *domainIn, char *domainOut);
	int     RestoreSharedValues();

	// this function returns the payload in the arrived subscription (on any topic)
	char	*ProcessMessage	(char *message);

	//		Helper functions for reading/writing variables forn C
	char	*GetVariable	(char *name);
	int		SetVariable		(char *name, char *value);

	int		SaveConnParams	(void);
	int 	RecConnParams	(void);
	int		Flush			(void);

	int		NEWCONN			(void);

	int 	SaveDevParamsToNVM(void);

	void	*GetSubspaceElement(const char *prefix, const char *id);
	unsigned int GetSubspaceElementsCount(const char* prefix);
	const char *GetSubspaceElementByIndex(const char *prefix, unsigned int n);


#ifdef __cplusplus
}
#endif








#endif /* NBINTERFACE_H_ */
