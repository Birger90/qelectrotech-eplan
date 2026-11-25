/*
	Copyright 2006-2026 The QElectroTech Team
	This file is part of QElectroTech.

	QElectroTech is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	QElectroTech is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with QElectroTech.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef THMLABELSHEET_H
#define THMLABELSHEET_H

#include "thmjob.h"

#include <QGraphicsView>
#include <QList>
#include <QStringList>

class QGraphicsScene;
class ThmLabelField;

/**
	@brief The labels drawn as they come off the printer: the rows of the
	material one under the other, each label at its place in the row with
	its text field, in true proportions (the profile of the article, which
	the maker's layout gives).

	Every label is a field of its own. Editable, the user types into it as
	in M-Print PRO: Tab or Return goes on to the next label, and there is
	always an empty label after the last text to go on writing.
	Read only, it shows the labels of the project.
*/
class ThmLabelSheet : public QGraphicsView
{
	Q_OBJECT
	public:
		explicit ThmLabelSheet(QWidget *parent = nullptr);

			/// The material: size and place of the labels and of their text
		void setProfile(const ThmJob::Profile &profile, bool known = true);
			/// One text per label, in printing order
		void setTexts(const QStringList &texts);
		QStringList texts() const;
		void setEditable(bool editable);
		bool isEditable() const { return m_editable; }
			/// Number of label fields drawn (texts plus the empty ones)
		int fieldCount() const { return int(m_fields.size()); }
			/// Labels drawn at most; the rest is only counted
		static constexpr int kMaxFields = 2000;

			/// Keyboard focus to field @p index (for typing and tests)
		void focusField(int index);

	signals:
		void textsChanged(const QStringList &texts);

	protected:
		void resizeEvent(QResizeEvent *event) override;
		void wheelEvent(QWheelEvent *event) override;
		void mousePressEvent(QMouseEvent *event) override;

	private:
		friend class ThmLabelField;
		void rebuild();
		void fieldEdited(ThmLabelField *field);
		void focusNext(ThmLabelField *field, int step);
		void fitWidth();
		void place(ThmLabelField *field);

		QGraphicsScene *m_scene = nullptr;
		ThmJob::Profile m_profile;
		bool m_known = true;
		QStringList m_texts;
		bool m_editable = false;
		QList<ThmLabelField *> m_fields;
		double m_zoom = 1;   ///< on top of the fit to the width
		bool m_building = false;
};

#endif // THMLABELSHEET_H
