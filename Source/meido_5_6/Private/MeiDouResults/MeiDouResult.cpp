#include "MeiDouResults/MeiDouResult.h"

void AMeiDouResult::OnResultEnded_Implementation()
{
	if (bEndsWithResult)
	{
		Destroy();
	}
}
