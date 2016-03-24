#pragma once
#include <sprite_nodes/CCSprite.h>
#include <vector>

// grid flag ------------------------------------------------------------------------------------
enum TL_GRID_FLAG
{
	TL_GRID_FLAG_PASS				    = 0x00000001,					// 能否通行
};

USING_NS_CC;

class TLMapBlock : public CCNode
{
public:
	TLMapBlock( const std::string& strMapBlockFile );
	virtual ~TLMapBlock();
    static TLMapBlock* create( const std::string& strFileName );
    static bool newMapBlock( const std::string& strMapBlockFile, int nRow, int nCol, int nGridWidth, int nGridHeight );

    virtual bool init();

public:
    bool load( const std::string& strFileName );

    void clear();

    // 下面的是编辑器独有的
#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
public:
    void create( int nRow, int nCol, int nWidth, int nHeight );
    void save();

    void recreateGridLine();
    void setIsShowGirdLine( bool bIsShow );
    bool getIsShowGridLine() const { return m_bShowGridLine; }

protected:
    std::string m_strMapBlockFile;
    int m_nRow, m_nCol;
    int m_nWidth, m_nHeight;

    struct SpriteInfo {
        CCSprite*       pSprite;
        std::string     strFileName;
        float           x, y;
        float           scale_x, scale_y;
        float           rotation;
    };
    std::list<SpriteInfo*> m_listAllSprites;
    std::vector<int> m_vecGridStates;

    bool m_bShowGridLine;
    CCSpriteBatchNode* m_pGridBatchNode;
#endif
};
