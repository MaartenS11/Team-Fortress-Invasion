#include "cbase.h"
#include "particles_simple.h"

class C_ResourceChunk : public C_BaseAnimating
{
	DECLARE_CLASS( C_ResourceChunk, C_BaseAnimating );
public:
	DECLARE_CLIENTCLASS();
};

IMPLEMENT_CLIENTCLASS_DT( C_ResourceChunk, DT_ResourceChunk, CResourceChunk )
END_RECV_TABLE()

