#include "stdafx.h"
#include "WeaselTSF.h"
#include "CandidateList.h"
#include "ResponseParser.h"

STDAPI WeaselTSF::DoEditSession(TfEditCookie ec)
{
	// get commit string from server
	std::wstring commit;
	weasel::Status status;
	weasel::Config config;
	auto context = std::make_shared<weasel::Context>();

	auto _NewComposition = [this, &config]() {
		_UpdateCompositionWindow(_pEditSessionContext);
		_StartComposition(_pEditSessionContext, _fCUASWorkaroundEnabled && !config.inline_preedit);
	};

	weasel::ResponseParser parser(&commit, context.get(), &status, &config, &_cand->style());

	bool ok = m_client.GetResponseData(std::ref(parser));

	_UpdateUI(*context, status);

	if (ok)
	{
		if (!commit.empty())
		{
			// For auto-selecting, commit and preedit can both exist.
			// Commit and close the original composition first.
			if (!_IsComposing()) {
				_NewComposition();
			}
			_InsertText(_pEditSessionContext, commit);
			_EndComposition(_pEditSessionContext, false);
		}
		if (status.composing && !_IsComposing())
		{
			_NewComposition();
		}
		else if (!status.composing && _IsComposing())
		{
			_EndComposition(_pEditSessionContext, true);
		}
		if (status.composing) {
			_UpdateCompositionWindow(_pEditSessionContext);
		}
		if (_IsComposing() && config.inline_preedit)
		{
			_ShowInlinePreedit(_pEditSessionContext, context);
		}
	}


	return TRUE;
}

